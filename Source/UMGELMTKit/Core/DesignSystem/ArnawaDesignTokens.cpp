// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArnawaDesignTokens.h"

FLinearColor UArnawaDesignTokens::GetColor(FName TokenName, FLinearColor Fallback) const
{
	if (const FLinearColor* Found = Colors.Find(TokenName))
	{
		return *Found;
	}
	return Fallback;
}

bool UArnawaDesignTokens::GetTypeRole(FName RoleName, FArnawaTypeRole& OutRole) const
{
	if (const FArnawaTypeRole* Found = TypeRoles.Find(RoleName))
	{
		OutRole = *Found;
		return true;
	}
	return false;
}
