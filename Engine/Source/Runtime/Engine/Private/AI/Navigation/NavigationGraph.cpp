// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#include "EnginePrivate.h"
#include "NavGraphGenerator.h"
#include "AI/NavDataGenerator.h"
#include "AI/Navigation/NavNodeInterface.h"
#include "AI/Navigation/NavigationGraphNodeComponent.h"
#include "AI/Navigation/NavigationGraphNode.h"
#include "AI/Navigation/NavigationGraph.h"

//----------------------------------------------------------------------//
// FNavGraphNode
//----------------------------------------------------------------------//
FNavGraphNode::FNavGraphNode() 
{
	Edges.Reserve(InitialEdgesCount);
}

//----------------------------------------------------------------------//
// UNavigationGraphNodeComponent
//----------------------------------------------------------------------//
UNavigationGraphNodeComponent::UNavigationGraphNodeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UNavigationGraphNodeComponent::BeginDestroy()
{
	Super::BeginDestroy();
	
	if (PrevNodeComponent != NULL)
	{
		PrevNodeComponent->NextNodeComponent = NextNodeComponent;
	}

	if (NextNodeComponent != NULL)
	{
		NextNodeComponent->PrevNodeComponent = PrevNodeComponent;
	}

	NextNodeComponent = NULL;
	PrevNodeComponent = NULL;
}

//----------------------------------------------------------------------//
// ANavigationGraphNode
//----------------------------------------------------------------------//
ANavigationGraphNode::ANavigationGraphNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

//----------------------------------------------------------------------//
// ANavigationGraph
//----------------------------------------------------------------------//

ANavigationGraph::ANavigationGraph(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

ANavigationData* ANavigationGraph::CreateNavigationInstances(UNavigationSystem* NavSys)
{
	if (NavSys == NULL || NavSys->GetWorld() == NULL)
	{
		return NULL;
	}

	// first check if there are any INavNodeInterface implementing actors in the world
	bool bCreateNavigation = false;
	FActorIterator It(NavSys->GetWorld());
	for(; It; ++It )
	{
		if (Cast<INavNodeInterface>(*It) != NULL)
		{
			bCreateNavigation = true;
			break;
		}
	}

	if (false && bCreateNavigation)
	{
		ANavigationGraph* Instance = NavSys->GetWorld()->SpawnActor<ANavigationGraph>();
	}

	return NULL;
}

#if WITH_NAVIGATION_GENERATOR
FNavDataGenerator* ANavigationGraph::ConstructGenerator(const FNavAgentProperties& AgentProps)
{
	return new FNavGraphGenerator(this);
}
#endif
