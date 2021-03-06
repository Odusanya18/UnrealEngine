// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "HAL/PThreadCriticalSection.h"
#include "GenericPlatform/GenericPlatformCriticalSection.h"
#include "HAL/PThreadRWLock.h"

typedef FPThreadsCriticalSection FCriticalSection;
typedef FSystemWideCriticalSectionNotImplemented FSystemWideCriticalSection;
typedef FPThreadsRWLock FRWLock;
