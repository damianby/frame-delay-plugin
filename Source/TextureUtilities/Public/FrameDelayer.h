// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MediaTexture.h"
#include "MediaPlayer.h"
#include "Engine/TextureRenderTarget2D.h"

#include "FrameDelayer.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class TEXTUREUTILITIES_API UFrameDelayer : public UObject
{
	GENERATED_BODY()
	

public:

	void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Initialize", Keywords = "frame delay init"), Category = "Frame Delaying")
		bool Initialize(const int32 NumFrames, UTextureRenderTarget2D*& OutTexture);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Feed Texture", Keywords = "frame delay feed"), Category = "Frame Delaying")
		bool FeedTexture(UTexture* Texture);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Values", Keywords = "frame delay"), Category = "Frame Delaying")
		void GetValues(int32& OutNumFrames, int32& OutBufferId);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is Initialized", Keywords = "frame delay"), Category = "Frame Delaying")
		bool IsInitialized() { return bIsInitialized; }



private:

	void CopyRT_RenderThread(UTexture* Source, UTexture* Target);

	void ClearBuffers();
	
	EPixelFormat BufferPixelFormat = EPixelFormat::PF_Unknown;
	FIntVector BufferSize = FIntVector(0,0,0);
	bool bBufferFed = false;
	
	

	TArray<UTextureRenderTarget2D*> TextureBuffers;
	int32 BufferId = 0;
	

	UTextureRenderTarget2D* OutputTexture;

	bool bIsInitialized = false;
};
