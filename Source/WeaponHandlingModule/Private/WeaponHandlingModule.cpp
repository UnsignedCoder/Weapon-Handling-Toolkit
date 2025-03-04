// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponHandlingModule.h"
#include "Logging.h"

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FWeaponHandlingModule"

void FWeaponHandlingModule::StartupModule()
{
}

void FWeaponHandlingModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWeaponHandlingModule, WeaponHandlingModule);