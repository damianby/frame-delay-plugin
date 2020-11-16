// Copyright Epic Games, Inc. All Rights Reserved.

#include "TextureUtilitiesBPLibrary.h"
#include "TextureUtilities.h"

UTextureUtilitiesBPLibrary::UTextureUtilitiesBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

bool UTextureUtilitiesBPLibrary::CreateFrameDelayer(const int32 NumFrames, UFrameDelayer*& FrameDelayer, UTextureRenderTarget2D*& OutTexture)
{
	FrameDelayer = NewObject<UFrameDelayer>();

	return FrameDelayer->Initialize(NumFrames, OutTexture);
}
