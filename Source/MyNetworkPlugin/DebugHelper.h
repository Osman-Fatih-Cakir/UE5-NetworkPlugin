#pragma once

#define TEST_PRINT() Debug::Print("HERE HERE HERE");

namespace Debug
{
	static FString Vector3ToString(const FVector& Vector)
	{
		return FString::Printf(TEXT("%s %s %s"), *FString::SanitizeFloat(Vector.X), *FString::SanitizeFloat(Vector.Y), *FString::SanitizeFloat(Vector.Z));
	}

	static void Print(const FString& Msg, const FColor& Color = FColor::MakeRandomColor(), float TimeToDisplay = 6.0f, int32 InKey = -1)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, TimeToDisplay, Color, Msg);
		}

		UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
	}

	static void PrintVec3(FVector vec, const FColor& Color = FColor::MakeRandomColor(), int32 InKey = -1)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, 6.f, Color, *Vector3ToString(vec));
		}

		UE_LOG(LogTemp, Warning, TEXT("%f %f %f"), vec.X, vec.Y, vec.Z);
	}


	static void PrintInt(int val, const FColor& Color = FColor::MakeRandomColor(), int32 InKey = -1)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, 6.f, Color, *FString::FromInt(val));
		}

		UE_LOG(LogTemp, Warning, TEXT("%d"), val);
	}

	static void PrintFloat(float val, const FColor& Color = FColor::MakeRandomColor(), int32 InKey = -1)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, 6.f, Color, *FString::SanitizeFloat(val));
		}

		UE_LOG(LogTemp, Warning, TEXT("%f"), val);
	}
}
