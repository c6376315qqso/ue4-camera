// Fill out your copyright notice in the Description page of Project Settings.

#include "Engine/TextureRenderTarget2D.h"
#include <vector>
#include <assert.h>
#include <fstream>
#include <cstdlib>
#include <time.h>
#include <string>
#include "Kismet/GameplayStatics.h"
#include "ImageUtils.h"
#include <direct.h>
#include <io.h>
#include "GenericPlatform/GenericPlatformMisc.h"

#include "panoramic_getor.h"

#define LENX 1024
#define LENY 1024
#define FOV 90

FBox2D Apanoramic_getor::GetBbox(USceneCaptureComponent2D* camera, person p) {
    TArray<FVector> Points;
    FVector Origin = p.loc, Extend = p.extend;
    Points.Add(Origin + FVector(Extend.X, Extend.Y, Extend.Z));
    Points.Add(Origin + FVector(-Extend.X, Extend.Y, Extend.Z));
    Points.Add(Origin + FVector(Extend.X, -Extend.Y, Extend.Z));
    Points.Add(Origin + FVector(-Extend.X, -Extend.Y, Extend.Z));
    Points.Add(Origin + FVector(Extend.X, Extend.Y, -Extend.Z));
    Points.Add(Origin + FVector(-Extend.X, Extend.Y, -Extend.Z));
    Points.Add(Origin + FVector(Extend.X, -Extend.Y, -Extend.Z));
    Points.Add(Origin + FVector(-Extend.X, -Extend.Y, -Extend.Z));
    FVector2D MinPixel(12345678, 12345678);
    FVector2D MaxPixel(0, 0);
    FIntPoint size(LENX, LENY);
    for (auto& Point : Points) {
        FVector2D Pixel;
        AMyGameState::ProjectWorldLocationToCapturedScreen(Camera, Point, size, Pixel);
        MaxPixel.X = FMath::Min(Pixel.X, MaxPixel.X);
        MaxPixel.Y = FMath::Min(Pixel.Y, MaxPixel.Y);
        MinPixel.X = FMath::Min(Pixel.X, MinPixel.X);
        MinPixel.Y = FMath::Min(Pixel.Y, MinPixel.Y);

    }
    return FBox2D(MinPixel, MaxPixel);

}

//// Sets default values
Apanoramic_getor::Apanoramic_getor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;
    RootComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Root"));

    Camera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    Camera->PrimaryComponentTick.TickGroup = PrimaryActorTick.TickGroup;

    CameraDepth = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("DCamera"));
    CameraDepth->SetupAttachment(RootComponent);
    CameraDepth->PrimaryComponentTick.TickGroup = PrimaryActorTick.TickGroup;

    CameraSeg = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SCamera"));
    CameraSeg->SetupAttachment(RootComponent);
    CameraSeg->PrimaryComponentTick.TickGroup = PrimaryActorTick.TickGroup;

    CameraBack = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("BCamera"));
    CameraBack->SetupAttachment(RootComponent);
    CameraBack->PrimaryComponentTick.TickGroup = PrimaryActorTick.TickGroup;

    ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VCamera"));
    ViewCamera->SetupAttachment(RootComponent);
}


bool match_people(FString name) {
    FRegexPattern pattern(TEXT("BP_[\\s\\S]*_[mf]_C"));
    FRegexMatcher matcher(pattern, name);

    return matcher.FindNext() || name.Find("spline_man") != -1 || name.Find("spline_woman") != -1;

}





// Called when the game starts or when spawned
void Apanoramic_getor::BeginPlay()
{
	Super::BeginPlay();
    if (GetWorld() == NULL) {
        UE_LOG(LogTemp, Warning, TEXT("No World"));
        return;
    }
    tot_people_cnt = 0;
    mystate = Cast<AMyGameState>(GetWorld()->GetGameState<AMyGameState>());

    RenderTarget = NewObject<UTextureRenderTarget2D>();
    RenderTargetSeg = NewObject<UTextureRenderTarget2D>();
    RenderTargetBack = NewObject<UTextureRenderTarget2D>();
    RenderTargetDepth = NewObject<UTextureRenderTarget2D>();
    

    RenderTarget->ClearColor = FLinearColor::Black;
    RenderTarget->InitCustomFormat(LENX, LENY, PF_B8G8R8A8, true);

    RenderTargetDepth->InitAutoFormat(LENX, LENY);
    RenderTargetDepth->RenderTargetFormat = RTF_R32f;

    RenderTargetSeg->ClearColor = FLinearColor::Black;
    RenderTargetSeg->InitCustomFormat(LENX, LENY, PF_B8G8R8A8, true);
    
    RenderTargetBack->ClearColor = FLinearColor::Black;
    RenderTargetBack->InitCustomFormat(LENX, LENY, PF_B8G8R8A8, true);

    ViewCamera->FieldOfView = FOV;

    CameraSeg->FOVAngle = FOV;
    CameraSeg->TextureTarget = RenderTargetSeg;
    CameraSeg->CaptureSource = SCS_FinalColorLDR;
    CameraSeg->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

    Camera->FOVAngle = FOV;
    Camera->TextureTarget = RenderTarget;
    Camera->CaptureSource = SCS_FinalColorLDR;


    CameraBack->FOVAngle = FOV;
    CameraBack->TextureTarget = RenderTargetBack;
    CameraBack->CaptureSource = SCS_FinalColorLDR;
    CameraBack->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;


    CameraDepth->FOVAngle = FOV;
    CameraDepth->TextureTarget = RenderTargetDepth;
    CameraDepth->CaptureSource = SCS_SceneDepth;

    Camera->bCaptureEveryFrame = false;
    CameraSeg->bCaptureEveryFrame = false;
    CameraBack->bCaptureEveryFrame = false;

    
    Camera->TextureTarget->TargetGamma = 3;
    CameraSeg->TextureTarget->TargetGamma = 3;
    CameraBack->TextureTarget->TargetGamma = 3;

    string folder("C:\\Users\\liule\\Desktop\\save\\");
    
    ofstream of(folder + "camera parameters.txt");
    FVector loc = Camera->GetComponentLocation(); 
    FQuat qua = Camera->GetComponentQuat();
    of << loc.X << " " << loc.Y << " " << loc.Z << "\n";
    of << qua.X << " " << qua.Y << " " << qua.Z << " " << qua.W;
}

void Apanoramic_getor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    string folder("C:\\Users\\liule\\Desktop\\save\\");
    ofstream of(folder + "bones.txt");
    for (auto& elem : all_bones) {
        of << elem.Key << "\n";                      // person id
        auto mp = elem.Value;
        of << mp.Num() << "\n";                      // frame num
        for (auto& elem2 : mp) {
            of << elem2.Key << "\n";                         // frame id 
            auto p = elem2.Value;
            auto p2 = all_bones2d[elem.Key][elem2.Key];
            for (int i = 0; i < p.Num(); i++) {
                of << p[i].X << " " << p[i].Y << " " << p[i].Z << " ";  // 3d loc
                of << p2[i].X << " " << p2[i].Y << "\n";                      // 2d loc
            }
        }
    }
}


void write_loc(int id, int x, int y, string & path) {
    ofstream of(path, ios::app);
    of << id << " " << x << " " << y << '\n';
}

TArray<FVector> get_bones(AActor* actor) {
    USkeletalMeshComponent* playermesh = Cast<USkeletalMeshComponent>(actor->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
    const int num = 19;
    string bone_names[num] = {string("Bip01-HeadNub"), string("Bip01-Head"), string("Bip01-L-UpperArm"), string("Bip01-R-UpperArm"), 
        string("Bip01-L-Forearm"), string("Bip01-R-Forearm"), string("Bip01-L-Hand"), string("Bip01-R-Hand"),
        string("Bip01-Neck"),string("Bip01-Spine2"), string("Bip01-Spine1"), string("Bip01-Spine"), 
        string("Bip01-Pelvis"), string("Bip01-L-Thigh"), string("Bip01-R-Thigh"), string("Bip01-L-Calf"), string("Bip01-R-Calf"), 
        string("Bip01-L-Foot"), string("Bip01-R-Foot")};
    TArray<FVector> ret;
    for (int i = 0; i < num; i++) {
        ret.Emplace(playermesh->GetBoneLocation(bone_names[i].c_str(), EBoneSpaces::WorldSpace));
    }
    return ret;

}

// Called every frame
void Apanoramic_getor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    if (frame_cnt < 5) {
        frame_cnt++;
        return;
    }
    Camera->CaptureScene();
    CameraBack->CaptureScene();
    //FlushRenderingCommands();
    FTextureRenderTargetResource* RenderResource = Camera->TextureTarget->GameThread_GetRenderTargetResource();

    FTextureRenderTargetResource* RenderResourceS = NULL;
    FTextureRenderTargetResource* RenderResourceB = CameraBack->TextureTarget->GameThread_GetRenderTargetResource();

    FString name = this->GetName();
    TArray<FColor> image, depth, back, seg;
    image.AddUninitialized(LENX * LENY);
    seg.AddUninitialized(LENX * LENY);
    back.AddUninitialized(LENX * LENY);
    depth.AddUninitialized(LENX * LENY);
    FIntPoint size = { LENX, LENY };



    RenderResourceB->ReadPixels(back);
    RenderResource->ReadPixels(image);

    int rela_cnt = frame_cnt - 4;

    //FlushRenderingCommands();

    string folder1("C:\\Users\\liule\\Desktop\\save\\prime\\");
    string folder2("C:\\Users\\liule\\Desktop\\save\\seg\\");
    string folder3("C:\\Users\\liule\\Desktop\\save\\back\\");
    string folder4("C:\\Users\\liule\\Desktop\\save\\loc\\");
    

    if (_access(folder1.c_str(), 0) == -1)	
        _mkdir(folder1.c_str());
    if (_access(folder2.c_str(), 0) == -1)
        _mkdir(folder2.c_str());
    if (_access(folder3.c_str(), 0) == -1)
        _mkdir(folder3.c_str());
    if (_access(folder4.c_str(), 0) == -1)
        _mkdir(folder4.c_str());


    //folder1 += string(TCHAR_TO_UTF8(*name)) + "_";
    //folder2 += string(TCHAR_TO_UTF8(*name)) + "_";
    //folder3 += string(TCHAR_TO_UTF8(*name)) + "_";

    string path_i, path_s, path_b;
    string rela_cnt_s = to_string(rela_cnt);
    while (rela_cnt_s.size() < 4) rela_cnt_s = "0" + rela_cnt_s;
    path_i = folder1 + "image_" + rela_cnt_s +  ".png";
 
    path_b = folder3 + to_string(rela_cnt) +  "_b.png";

    FString CaptureFilename = FString(path_i.c_str());
    TArray<uint8> CompressedBitmap;
    FImageUtils::CompressImageArray(LENX, LENY, image, CompressedBitmap);
    FFileHelper::SaveArrayToFile(CompressedBitmap, *CaptureFilename);


    CaptureFilename = FString(path_b.c_str());
    TArray<uint8> CompressedBitmapB;
    FImageUtils::CompressImageArray(LENX, LENY, back, CompressedBitmapB);
    FFileHelper::SaveArrayToFile(CompressedBitmapB, *CaptureFilename);
    
    int cnt = 0;
    for (TActorIterator<AActor> actor(GetWorld()); actor; ++actor)
    {
        FVector2D inloc_seg, inloc_pri;
        bool okseg = AMyGameState::ProjectWorldLocationToCapturedScreen(CameraSeg, actor->GetActorLocation(), size, inloc_seg);
        bool okpri = AMyGameState::ProjectWorldLocationToCapturedScreen(Camera, actor->GetActorLocation(), size, inloc_pri);
        if (okseg && inloc_seg.X >= 0 && inloc_seg.X <= LENX && inloc_seg.Y >= 0 && inloc_seg.Y <= LENY) {
            if (match_people(actor->GetClass()->GetName())) {
                TArray<AActor*> showactors;
                showactors.Add(*actor);
                cnt++;
                if (mapid.Find(actor->GetUniqueID()) == NULL) {
                    mapid.Add(actor->GetUniqueID(), ++tot_people_cnt);
                    TMap<int32, TArray<FVector>> t1;
                    TMap<int32, TArray<FVector2D>> t2;
                    all_bones.Emplace(tot_people_cnt, t1);
                    all_bones2d.Emplace(tot_people_cnt, t2);
                } 
                int actor_cnt = mapid[actor->GetUniqueID()];
                UE_LOG(LogTemp, Warning, TEXT("matched %s"), *actor->GetClass()->GetName());
                CameraSeg->ShowOnlyActors = showactors;
                CameraSeg->CaptureScene();
                FlushRenderingCommands();
                RenderResourceS = CameraSeg->TextureTarget->GameThread_GetRenderTargetResource();
                RenderResourceS->ReadPixels(seg);
                path_s = folder2;
                path_s += to_string(actor_cnt);
                path_s += "_" + to_string(rela_cnt) + "_s.png";

                CaptureFilename = FString(path_s.c_str());
                TArray<uint8> CompressedBitmapS;
                FImageUtils::CompressImageArray(LENX, LENY, seg, CompressedBitmapS);
                FFileHelper::SaveArrayToFile(CompressedBitmapS, *CaptureFilename);
                
                TArray<FVector> bones = get_bones(*actor);
                TArray<FVector2D> bones_2d;
                all_bones[mapid[actor->GetUniqueID()]].Emplace(rela_cnt, bones);
             
                for (auto bone_loc : bones) {
                    FVector2D bone_loc2d;
                    AMyGameState::ProjectWorldLocationToCapturedScreen(CameraSeg, bone_loc, size, bone_loc2d);
                    bones_2d.Emplace(bone_loc2d);
                }
                all_bones2d[mapid[actor->GetUniqueID()]].Emplace(rela_cnt, bones_2d);
                string path_l = folder4;
                path_l += "loc_s" + to_string(actor_cnt) + "_" + to_string(rela_cnt) + ".txt";

                write_loc(actor->GetUniqueID(), inloc_seg.X, inloc_seg.Y, path_l);

               // if (cnt > 10) break;

            }
            string path_l = folder4;
            path_l += "loc" + to_string(rela_cnt) + ".txt";
            write_loc(actor->GetUniqueID(), inloc_pri.X, inloc_pri.Y, path_l);

        }

        
    }
    if (rela_cnt >= 50) {
        GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
    }

    frame_cnt++;

}

