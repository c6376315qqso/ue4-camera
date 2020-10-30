// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameState.h"


AMyGameState::AMyGameState() {
	memset(occupyed, 0, sizeof(occupyed));
	memset(timeused, 0, sizeof(timeused));
	memset(lastuse, 0, sizeof(lastuse));
	float rotate[RotatorNum][3];
	//float rotate[RotatorNum][3] = { 
	//	{0, 10, 100},{0, 10, 70}, 
	//	{0, 10, 100}, {0, -10, 100},
	//	{0, -10, 70},{0,-10,120}, 
	//	{0, -30, 100}, {0, -30, 100}, 
	//	{0, 0, 100}, {0, 0, 100}, 
	//	{0, 0, 100}, {0, -60, 100}, 
	//	{0, -60, 100}, {0, -60, 100}, 
	//	{0, -30, 100}};
	int minn[3], maxn[3];
	minn[1] = -30, maxn[1] = 0;
	minn[2] = 60, maxn[2] = 120;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 5; j++) {
			int ind = i * 5 + j;
			rotate[ind][0] = 0;
			rotate[ind][1] = minn[1] + 10 * i;
			rotate[ind][2] = minn[2] + 10 * j;
		}
	}
	for (int i = 0; i < RotatorNum; i++) {
		RotatorList[i].Pitch = rotate[i][1];
		RotatorList[i].Roll = rotate[i][0];
		RotatorList[i].Yaw = rotate[i][2];
	}
	camera_num = 0;

}

int AMyGameState::getNextRotationId()
{
	int minn = 0; 
	for (int i = 0; i < RotatorNum; i++) {
		if (lastuse[i] < lastuse[minn]) minn = i;
	}

	return minn;
}

bool AMyGameState::ProjectWorldLocationToCapturedScreen(
	USceneCaptureComponent2D* InCaptureComponent,
	const FVector& InWorldLocation,
	const FIntPoint& InRenderTarget2DSize,
	FVector2D& OutPixel
)
{
	// Render Target's Rectangle
	verify(InRenderTarget2DSize.GetMin() > 0);
	const FIntRect renderTargetRect(0, 0, InRenderTarget2DSize.X, InRenderTarget2DSize.Y);

	// Initialise Viewinfo for projection matrix from [InCaptureComponent]
	FMinimalViewInfo minimalViewInfo;
	InCaptureComponent->GetCameraView(0.f, minimalViewInfo);

	// Fetch [captureComponent]'s [CustomProjectionMatrix]
	TOptional<FMatrix> customProjectionMatrix;
	if (InCaptureComponent->bUseCustomProjectionMatrix) {
		customProjectionMatrix = InCaptureComponent->CustomProjectionMatrix;
	}

	// Calculate [cameraViewProjectionMatrix]
	FMatrix captureViewMatrix, captureProjectionMatrix, captureViewProjectionMatrix;
	UGameplayStatics::CalculateViewProjectionMatricesFromMinimalView(minimalViewInfo, customProjectionMatrix,
		captureViewMatrix, captureProjectionMatrix, captureViewProjectionMatrix);

	bool result = FSceneView::ProjectWorldToScreen(InWorldLocation, renderTargetRect,
		captureViewProjectionMatrix, OutPixel);

	UE_LOG(LogTemp, Verbose, TEXT("ON [%s] CAPTURED SCREEN: WORLD LOCATION [%s] HAS LOCAL PIXEL COORDINATES: (X) %lf [over %d] OR (Y) %lf [over %d]"),
		*InCaptureComponent->GetName(), *InWorldLocation.ToString(),
		OutPixel.X, InRenderTarget2DSize.X,
		OutPixel.Y, InRenderTarget2DSize.Y);
	return result;
}