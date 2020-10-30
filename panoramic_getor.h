#pragma once
#include "Components/SphereComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include <vector>
#include "MyGameState.h"
#include "Internationalization/Regex.h"
#include "EngineUtils.h"
#include "Camera/CameraComponent.h"
#include <time.h>
#include <map>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "panoramic_getor.generated.h"

using namespace std;

UCLASS()
class CITYPARK2_API Apanoramic_getor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	Apanoramic_getor();
	USceneCaptureComponent2D* Camera;
	USceneCaptureComponent2D* CameraSeg;
	USceneCaptureComponent2D* CameraBack;
	USceneCaptureComponent2D* CameraDepth;

	UCameraComponent* ViewCamera;
	UTextureRenderTarget2D* RenderTargetDepth;
	UTextureRenderTarget2D* RenderTargetSeg;
	UTextureRenderTarget2D* RenderTarget;
	UTextureRenderTarget2D* RenderTargetBack;
	int tot_people_cnt;
	TMap<int32, int32> mapid;
	UTexture2D* Texture2D;
	int lenx, leny;
	clock_t time1;

	TMap< int32, TMap<int32, TArray<FVector> > > all_bones;
	TMap< int32, TMap<int32, TArray<FVector2D> > > all_bones2d;

	FBox2D GetBbox(USceneCaptureComponent2D* camera, AActor* p);
	int rotate_id;
	int frame_cnt;
	AMyGameState* mystate;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};