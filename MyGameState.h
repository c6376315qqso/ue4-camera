// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include <map>
#include "Kismet/GameplayStatics.h"

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameState.generated.h"

using namespace std;
/**
 * 
 */
struct person
{
	FVector loc, extend;
	bool is_move;

};





UCLASS()
class CITYPARK2_API AMyGameState : public AGameStateBase
{
	GENERATED_BODY()


public:
	AMyGameState();
	static bool ProjectWorldLocationToCapturedScreen(
		USceneCaptureComponent2D* InCaptureComponent,
		const FVector& InWorldLocation,
		const FIntPoint& InRenderTarget2DSize,
		FVector2D& OutPixel
	);
	const static int RotatorNum = 15;
	int camera_num;
	FRotator RotatorList[RotatorNum];
	int occupyed[RotatorNum];
	int timeused[RotatorNum];
	int lastuse[RotatorNum];
	int getNextRotationId();
	map<int, person> people;
};

//srand((unsigned)time(NULL));