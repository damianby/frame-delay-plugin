
// Fill out your copyright notice in the Description page of Project Settings.


#include "FrameDelayer.h"


void UFrameDelayer::CopyRT_RenderThread(UTexture* Source, UTexture* Target)
{
	check(IsInRenderingThread());
	// Get global RHI command list 
	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();
	if (Source && Target && Target->Resource && Source->Resource && Target->Resource->IsInitialized() && Source->Resource->IsInitialized())
	{
		
		FTextureRHIRef sourceTexture = TRefCountPtr<FRHITexture>(Source->Resource->TextureRHI);
		
		FTextureRHIRef destTexture = TRefCountPtr<FRHITexture>(Target->Resource->TextureRHI);

		FResolveParams param;
		RHICmdList.CopyToResolveTarget(sourceTexture, destTexture, param);
	}
}

void UFrameDelayer::ClearBuffers()
{
	for (int i = 0; i < TextureBuffers.Num(); i++)
	{
		TextureBuffers[i]->RemoveFromRoot();
		TextureBuffers[i]->ConditionalBeginDestroy();
	}

	if (OutputTexture)
	{
		OutputTexture->RemoveFromRoot();
		OutputTexture->ConditionalBeginDestroy();
	}
}





bool UFrameDelayer::FeedTexture(UTexture* Texture)
{

	if (BufferId >= TextureBuffers.Num())
	{
		return false;
	}

	//Check if valid!
	if (!Texture || !Texture->Resource || !Texture->Resource->TextureRHI || !bIsInitialized)
	{
		return false;
	}

	FTextureRHIRef& TexRef = Texture->Resource->TextureRHI;
	FIntVector MediaTexSize = TexRef->GetSizeXYZ();

	if (TexRef->GetFormat() != BufferPixelFormat || MediaTexSize != BufferSize)
	{
		for (int i = 0; i < TextureBuffers.Num(); i++)
		{
			TextureBuffers[i]->ReleaseResource();
			TextureBuffers[i]->InitCustomFormat(MediaTexSize.X, MediaTexSize.Y, TexRef->GetFormat(), false);

		}

		OutputTexture->ReleaseResource();
		OutputTexture->InitCustomFormat(MediaTexSize.X, MediaTexSize.Y, TexRef->GetFormat(), false);

		BufferPixelFormat = TexRef->GetFormat();
		BufferSize = MediaTexSize;
	}

	UTexture* WriteBuffer;

	int32 LocalBufferId = BufferId;

	if (!bBufferFed)
	{
		WriteBuffer = TextureBuffers[LocalBufferId];

		ENQUEUE_RENDER_COMMAND(CopyTextureCmd)(
			[this, Texture, WriteBuffer](FRHICommandListImmediate& RHICmdList)
			{
				this->CopyRT_RenderThread(Texture, WriteBuffer);
			});

		BufferId++;
		if (BufferId >= TextureBuffers.Num())
		{
			bBufferFed = true;
			BufferId = BufferId % TextureBuffers.Num();
		}
	}
	else
	{
		WriteBuffer = TextureBuffers[LocalBufferId];

		ENQUEUE_RENDER_COMMAND(CopyTextureCmd1)(
			[this, WriteBuffer](FRHICommandListImmediate& RHICmdList)
			{
				this->CopyRT_RenderThread(WriteBuffer, this->OutputTexture);
				//RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThreadFlushResources);
			});

		ENQUEUE_RENDER_COMMAND(CopyTextureCmd2)(
			[this, Texture, WriteBuffer](FRHICommandListImmediate& RHICmdList)
			{
				this->CopyRT_RenderThread(Texture, WriteBuffer);
			});

		BufferId++;
		if (BufferId >= TextureBuffers.Num())
		{
			BufferId = BufferId % TextureBuffers.Num();
		}
	}

	return true;



	//IRendererModule* RendererModule = &FModuleManager::GetModuleChecked<IRendererModule>("Renderer");

	//ENQUEUE_RENDER_COMMAND(CopyTextureCmd)(
	//	[this, MediaTexture](FRHICommandListImmediate& RHICmdList)
	//	{
	//		
	//		

	//		RHICmdList.CopyToResolveTarget(
	//			MediaTexture->Resource->TextureRHI,	// Source texture
	//			OutputTexture->Resource->TextureRHI,
	//			FResolveParams());

	//		//FRHICopyTextureInfo CopyInfo;
	//		//RHICmdList.CopyTexture(MediaTexture->Resource->TextureRHI, RT->Resource->TextureRHI, CopyInfo);

	//		//CopyTextureTargets_RenderThread(MediaTexture->Resource->TextureRHI, RT->Resource->TextureRHI);

	//	});

	//ENQUEUE_RENDER_COMMAND(FRHICommandCopyTexture)([
	//	MediaTexture, RT, this
	//](FRHICommandListImmediate & RHICmdList)
	//{
	//		//RHICmdList.CopyTexture(MediaTexture->Resource->TextureRHI, RT->Resource->TextureRHI, FRHICopyTextureInfo());

	//		//CopyTextureTargets_RenderThread(MediaTexture->Resource->, RT);
	//});


	//FRHICopyTextureInfo CopyTexture;

	//IRendererModule* RendererModule = &FModuleManager::GetModuleChecked<IRendererModule>("Renderer");
	//FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();


	
	/*ENQUEUE_RENDER_COMMAND(FRHICommandCopyTexture)(MediaTexture->Resource, TextureRenderTarget->Resource, CopyTexture)
		{

		});*/

	//MediaTexture->Resource
	//ENQUEUE_UNIQUE_RENDER_COMMAND(void, FTexture2DRHIRef, InputTexture, InputTexture,
	//	{
	//		FRHIResourceCreateInfo CreateInfo;
	//		InputTexture = RHICreateTexture2D(960, 540, PF_R32_UINT, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
	//		// Write the contents of the texture.
	//		uint32 DestStride;
	//		uint32 * DestBuffer = (uint32*)RHILockTexture2D(InputTexture, 0, RLM_WriteOnly, DestStride, false);
	//		int BufferSize = 960 * 540 * 4;
	//		uint8 * SourceBuffer = new uint8[BufferSize];
	//		for (int i = 0; i < BufferSize; i++) {
	//			SourceBuffer[i] = 255;
	//		}
	//		FMemory::Memcpy(DestBuffer, SourceBuffer, BufferSize);
	//		RHIUnlockTexture2D(InputTexture, 0, false);
	//	}
	//);

	return false;
}

void UFrameDelayer::GetValues(int32& OutNumFrames, int32& OutBufferId)
{
	OutNumFrames = TextureBuffers.Num();
	OutBufferId = BufferId;
}



void UFrameDelayer::BeginDestroy()
{
	ClearBuffers();

	Super::BeginDestroy();
}

bool UFrameDelayer::Initialize(const int32 NumFrames, UTextureRenderTarget2D*& OutTexture)
{
	ClearBuffers();
	for (int i = 0; i < NumFrames; i++)
	{
		UTextureRenderTarget2D* NewBuffer = NewObject<UTextureRenderTarget2D>();
		NewBuffer->AddToRoot();
		TextureBuffers.Add(NewBuffer);
	}

	UTextureRenderTarget2D* NewOutTexture = NewObject<UTextureRenderTarget2D>();
	NewOutTexture->AddToRoot();

	OutTexture = OutputTexture = NewOutTexture;

	bIsInitialized = true;

	return true;
}
