// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SharedMapView.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAssetData, Log, All);
/** A class to hold important information about an assets found by the Asset Registry */
class FAssetData
{
public:

	/** The object path for the asset in the form 'Package.GroupNames.AssetName' */
	FName ObjectPath;
	/** The name of the package in which the asset is found */
	FName PackageName;
	/** The path to the package in which the asset is found */
	FName PackagePath;
	/** The '.' delimited list of group names in which the asset is found. NAME_None if there were no groups */
	FName GroupNames;
	/** The name of the asset without the package or groups */
	FName AssetName;
	/** The name of the asset's class */
	FName AssetClass;
	/** The map of values for properties that were marked AssetRegistrySearchable */
	TSharedMapView<FName, FString> TagsAndValues;
	/** The IDs of the chunks this asset is located in for streaming install.  Empty if not assigned to a chunk */
	TArray<int32> ChunkIDs;
	/** Asset package flags */
	uint32 PackageFlags;

public:
	/** Default constructor */
	FAssetData()
	{}

	/** Constructor */
	FAssetData(FName InPackageName, FName InPackagePath, FName InGroupNames, FName InAssetName, FName InAssetClass, TMap<FName, FString> InTags, TArray<int32> InChunkIDs, uint32 InPackageFlags)
		: PackageName(InPackageName)
		, PackagePath(InPackagePath)
		, GroupNames(InGroupNames)
		, AssetName(InAssetName)
		, AssetClass(InAssetClass)
		, TagsAndValues(MakeSharedMapView(MoveTemp(InTags)))
		, ChunkIDs(MoveTemp(InChunkIDs))
		, PackageFlags(InPackageFlags)
	{
		FString ObjectPathStr = PackageName.ToString() + TEXT(".");

		if ( GroupNames != NAME_None )
		{
			ObjectPathStr += GroupNames.ToString() + TEXT(".");
		}

		ObjectPathStr += AssetName.ToString();

		ObjectPath = FName(*ObjectPathStr);
	}

	/** Constructor taking a UObject */
	FAssetData(const UObject* InAsset)
	{
		if ( InAsset != NULL )
		{
			const UClass* InClass = Cast<UClass>(InAsset);
			if (InClass && InClass->ClassGeneratedBy)
			{
				// this UObject was generated by another class, use the generator to identify
				// the source asset:
				InAsset = InClass->ClassGeneratedBy;
			}

			const UPackage* Outermost = InAsset->GetOutermost();

			// Find the group names
			FString GroupNamesStr;
			FString AssetNameStr;
			InAsset->GetPathName(Outermost).Split(TEXT("."), &GroupNamesStr, &AssetNameStr, ESearchCase::CaseSensitive);

			PackageName = Outermost->GetFName();
			PackagePath = FName(*FPackageName::GetLongPackagePath(Outermost->GetName()));
			GroupNames = FName(*GroupNamesStr);
			AssetName = InAsset->GetFName();
			AssetClass = InAsset->GetClass()->GetFName();
			ObjectPath = FName(*InAsset->GetPathName());

			TArray<UObject::FAssetRegistryTag> TagList;
			InAsset->GetAssetRegistryTags(TagList);

			TMap<FName, FString> TagsAndValuesMap;
			for ( auto TagIt = TagList.CreateConstIterator(); TagIt; ++TagIt )
			{
				TagsAndValuesMap.Add(TagIt->Name, TagIt->Value);
			}
			TagsAndValues = MakeSharedMapView(MoveTemp(TagsAndValuesMap));

			ChunkIDs = Outermost->GetChunkIDs();
			PackageFlags = Outermost->GetPackageFlags();
		}
	}

	/** FAssetDatas are equal if their object paths match */
	bool operator==(const FAssetData& Other) const
	{
		return ObjectPath == Other.ObjectPath;
	}

	bool operator!=(const FAssetData& Other) const
	{
		return ObjectPath != Other.ObjectPath;
	}

	bool operator>(const FAssetData& Other) const
	{
		return ObjectPath > Other.ObjectPath;
	}

	bool operator<(const FAssetData& Other) const
	{
		return ObjectPath < Other.ObjectPath;
	}

	/** Checks to see if this AssetData refers to an asset or is NULL */
	bool IsValid() const
	{
		return ObjectPath != NAME_None;
	}

	/** Returns true if this asset was found in a UAsset file */
	bool IsUAsset() const
	{
		return FPackageName::GetLongPackageAssetName(PackageName.ToString()) == AssetName.ToString();
	}

	/** Returns the full name for the asset in the form: Class ObjectPath */
	FString GetFullName() const
	{
		FString FullName;
		GetFullName(FullName);
		return FullName;
	}

	/** Populates OutFullName with the full name for the asset in the form: Class ObjectPath */
	void GetFullName(FString& OutFullName) const
	{
		OutFullName.Reset();
		AssetClass.AppendString(OutFullName);
		OutFullName.AppendChar(' ');
		ObjectPath.AppendString(OutFullName);
	}

	/** Returns the name for the asset in the form: Class'ObjectPath' */
	FString GetExportTextName() const
	{
		FString ExportTextName;
		GetExportTextName(ExportTextName);
		return ExportTextName;
	}

	/** Populates OutExportTextName with the name for the asset in the form: Class'ObjectPath' */
	void GetExportTextName(FString& OutExportTextName) const
	{
		OutExportTextName.Reset();
		AssetClass.AppendString(OutExportTextName);
		OutExportTextName.AppendChar('\'');
		ObjectPath.AppendString(OutExportTextName);
		OutExportTextName.AppendChar('\'');
	}

	/** Returns true if the this asset is a redirector. */
	bool IsRedirector() const
	{
		if ( AssetClass == UObjectRedirector::StaticClass()->GetFName() )
		{
			return true;
		}

		return false;
	}

	/** Returns the class UClass if it is loaded. It is not possible to load the class if it is unloaded since we only have the short name. */
	UClass* GetClass() const
	{
		if ( !IsValid() )
		{
			// Dont even try to find the class if the objectpath isn't set
			return NULL;
		}

		UClass* FoundClass = FindObject<UClass>(ANY_PACKAGE, *AssetClass.ToString());

		if (!FoundClass)
		{
			// Look for class redirectors
			FName NewPath = FLinkerLoad::FindNewNameForClass(AssetClass, false);

			if (NewPath != NAME_None)
			{
				FoundClass = FindObject<UClass>(ANY_PACKAGE, *NewPath.ToString());
			}
		}
		return FoundClass;
	}

	/** Convert to a StringAssetReference for loading */
	struct FStringAssetReference ToStringReference() const
	{
		return FStringAssetReference(ObjectPath.ToString());
	}

	/** Returns the asset UObject if it is loaded or loads the asset if it is unloaded then returns the result */
	UObject* GetAsset() const
	{
		if ( !IsValid() )
		{
			// Dont even try to find the object if the objectpath isn't set
			return NULL;
		}

		UObject* Asset = FindObject<UObject>(NULL, *ObjectPath.ToString());
		if ( Asset == NULL )
		{
			Asset = LoadObject<UObject>(NULL, *ObjectPath.ToString());
		}

		return Asset;
	}

	UPackage* GetPackage() const
	{
		if (PackageName == NAME_None)
		{
			return NULL;
		}

		UPackage* Package = FindPackage(NULL, *PackageName.ToString());
		if (Package)
		{
			Package->FullyLoad();
		}
		else
		{
			Package = LoadPackage(NULL, *PackageName.ToString(), LOAD_None);
		}

		return Package;
	}

	/** Try and get the value associated with the given tag as a type converted value */
	template <typename ValueType>
	bool GetTagValue(const FName InTagName, ValueType& OutTagValue) const;

	/** Try and get the value associated with the given tag as a type converted value, or an empty value if it doesn't exist */
	template <typename ValueType>
	ValueType GetTagValueRef(const FName InTagName) const;

	/** Returns true if the asset is loaded */
	bool IsAssetLoaded() const
	{
		return IsValid() && FindObject<UObject>(NULL, *ObjectPath.ToString()) != NULL;
	}

	/** Prints the details of the asset to the log */
	void PrintAssetData() const
	{
		UE_LOG(LogAssetData, Log, TEXT("    FAssetData for %s"), *ObjectPath.ToString());
		UE_LOG(LogAssetData, Log, TEXT("    ============================="));
		UE_LOG(LogAssetData, Log, TEXT("        PackageName: %s"), *PackageName.ToString());
		UE_LOG(LogAssetData, Log, TEXT("        PackagePath: %s"), *PackagePath.ToString());
		UE_LOG(LogAssetData, Log, TEXT("        GroupNames: %s"), *GroupNames.ToString());
		UE_LOG(LogAssetData, Log, TEXT("        AssetName: %s"), *AssetName.ToString());
		UE_LOG(LogAssetData, Log, TEXT("        AssetClass: %s"), *AssetClass.ToString());
		UE_LOG(LogAssetData, Log, TEXT("        TagsAndValues: %d"), TagsAndValues.Num());

		for (const auto& TagValue: TagsAndValues)
		{
			UE_LOG(LogAssetData, Log, TEXT("            %s : %s"), *TagValue.Key.ToString(), *TagValue.Value);
		}

		UE_LOG(LogAssetData, Log, TEXT("        ChunkIDs: %d"), ChunkIDs.Num());

		for (int32 Chunk: ChunkIDs)
		{
			UE_LOG(LogAssetData, Log, TEXT("                 %d"), Chunk);
		}

		UE_LOG(LogAssetData, Log, TEXT("        PackageFlags: %d"), PackageFlags);
	}

	/** Get the first FAssetData of a particular class from an Array of FAssetData */
	static FAssetData GetFirstAssetDataOfClass(const TArray<FAssetData>& Assets, const UClass* DesiredClass)
	{
		for(int32 AssetIdx=0; AssetIdx<Assets.Num(); AssetIdx++)
		{
			const FAssetData& Data = Assets[AssetIdx];
			UClass* AssetClass = Data.GetClass();
			if( AssetClass != NULL && AssetClass->IsChildOf(DesiredClass) )
			{
				return Data;
			}
		}
		return FAssetData();
	}

	/** Convenience template for finding first asset of a class */
	template <class T>
	static T* GetFirstAsset(const TArray<FAssetData>& Assets)
	{
		UClass* DesiredClass = T::StaticClass();
		UObject* Asset = FAssetData::GetFirstAssetDataOfClass(Assets, DesiredClass).GetAsset();
		check(Asset == NULL || Asset->IsA(DesiredClass));
		return (T*)Asset;
	}

	/** Operator for serialization */
	friend FArchive& operator<<(FArchive& Ar, FAssetData& AssetData)
	{
		// serialize out the asset info
		Ar << AssetData.ObjectPath;
		Ar << AssetData.PackagePath;
		Ar << AssetData.AssetClass;
		Ar << AssetData.GroupNames;

		// these are derived from ObjectPath, probably can skip serializing at the expense of runtime string manipulation
		Ar << AssetData.PackageName;
		Ar << AssetData.AssetName;

		Ar << const_cast<TMap<FName, FString>&>(AssetData.TagsAndValues.GetMap());

		if (Ar.UE4Ver() >= VER_UE4_CHANGED_CHUNKID_TO_BE_AN_ARRAY_OF_CHUNKIDS)
		{
			Ar << AssetData.ChunkIDs;
		}
		else if (Ar.UE4Ver() >= VER_UE4_ADDED_CHUNKID_TO_ASSETDATA_AND_UPACKAGE)
		{
			// loading old assetdata.  we weren't using this value yet, so throw it away
			int ChunkID = -1;
			Ar << ChunkID;
		}

		if (Ar.UE4Ver() >= VER_UE4_COOKED_ASSETS_IN_EDITOR_SUPPORT)
		{
			Ar << AssetData.PackageFlags;
		}

		return Ar;
	}

private:
	bool GetTagValueStringImpl(const FName InTagName, FString& OutTagValue) const
	{
		if (const FString* FoundValue = TagsAndValues.Find(InTagName))
		{
			bool bIsHandled = false;
			if (FTextStringHelper::IsComplexText(**FoundValue))
			{
				FText TmpText;
				if (FTextStringHelper::ReadFromString(**FoundValue, TmpText))
				{
					bIsHandled = true;
					OutTagValue = TmpText.ToString();
				}
			}

			if (!bIsHandled)
			{
				OutTagValue = *FoundValue;
			}

			return true;
		}

		return false;
	}

	bool GetTagValueTextImpl(const FName InTagName, FText& OutTagValue) const
	{
		if (const FString* FoundValue = TagsAndValues.Find(InTagName))
		{
			if (!FTextStringHelper::ReadFromString(**FoundValue, OutTagValue))
			{
				OutTagValue = FText::FromString(*FoundValue);
			}
			return true;
		}
		return false;
	}

	bool GetTagValueNameImpl(const FName InTagName, FName& OutTagValue) const
	{
		FString StrValue;
		if (GetTagValueStringImpl(InTagName, StrValue))
		{
			OutTagValue = *StrValue;
			return true;
		}
		return false;
	}
};

template <typename ValueType>
inline bool FAssetData::GetTagValue(const FName InTagName, ValueType& OutTagValue) const
{
	if (const FString* FoundValue = TagsAndValues.Find(InTagName))
	{
		FMemory::Memzero(&OutTagValue, sizeof(ValueType));
		LexicalConversion::FromString(OutTagValue, **FoundValue);
		return true;
	}
	return false;
}

template <>
inline bool FAssetData::GetTagValue<FString>(const FName InTagName, FString& OutTagValue) const
{
	return GetTagValueStringImpl(InTagName, OutTagValue);
}

template <>
inline bool FAssetData::GetTagValue<FText>(const FName InTagName, FText& OutTagValue) const
{
	return GetTagValueTextImpl(InTagName, OutTagValue);
}

template <>
inline bool FAssetData::GetTagValue<FName>(const FName InTagName, FName& OutTagValue) const
{
	return GetTagValueNameImpl(InTagName, OutTagValue);
}

template <typename ValueType>
inline ValueType FAssetData::GetTagValueRef(const FName InTagName) const
{
	ValueType TmpValue;
	FMemory::Memzero(&TmpValue, sizeof(ValueType));
	if (const FString* FoundValue = TagsAndValues.Find(InTagName))
	{
		LexicalConversion::FromString(TmpValue, **FoundValue);
	}
	return TmpValue;
}

template <>
inline FString FAssetData::GetTagValueRef<FString>(const FName InTagName) const
{
	FString TmpValue;
	GetTagValueStringImpl(InTagName, TmpValue);
	return TmpValue;
}

template <>
inline FText FAssetData::GetTagValueRef<FText>(const FName InTagName) const
{
	FText TmpValue;
	GetTagValueTextImpl(InTagName, TmpValue);
	return TmpValue;
}

template <>
inline FName FAssetData::GetTagValueRef<FName>(const FName InTagName) const
{
	FName TmpValue;
	GetTagValueNameImpl(InTagName, TmpValue);
	return TmpValue;
}

/** A structure defining a thing that can be reference bt something else in the asset registry */
struct FAssetIdentifier
{
	/** The name of the package that is depended on, this is always set */
	FName PackageName;
	/** Specific object within a package. If empty, assumed to be the default asset */
	FName ObjectName;
	/** Name of specific value being referenced, if ObjectName specifies a type such as a UStruct */
	FName ValueName;

	/** Can be implicitly constructed from just the package name */
	FAssetIdentifier(FName InPackageName, FName InObjectName = NAME_None, FName InValueName = NAME_None)
		: PackageName(InPackageName), ObjectName(InObjectName), ValueName(InValueName)
	{}

	FAssetIdentifier(UObject* SourceObject, FName InValueName)
	{
		if (SourceObject)
		{
			UPackage* Package = SourceObject->GetOutermost();
			PackageName = Package->GetFName();
			ObjectName = SourceObject->GetFName();
			ValueName = InValueName;
		}
	}

	FAssetIdentifier()
		: PackageName(NAME_None), ObjectName(NAME_None), ValueName(NAME_None)
	{}

	/** Returns true if this represents a specific value */
	bool IsValue() const
	{
		return ValueName != NAME_None;
	}

	/** Returns true if this represents a specific value */
	bool IsObject() const
	{
		return ObjectName != NAME_None && !IsValue();
	}

	/** Returns string version of this identifier in Package.Object::Name format */
	FString ToString() const
	{
		FString Result = PackageName.ToString();
		if (ObjectName != NAME_None)
		{
			Result += TEXT(".");
			Result += ObjectName.ToString();
		}
		if (ValueName != NAME_None)
		{
			Result += TEXT("::");
			Result += ValueName.ToString();
		}
		return Result;
	}

	/** Converts from Package.Object::Name format */
	static FAssetIdentifier FromString(const FString& String)
	{
		// To right of :: is value
		FString PackageString;
		FString ObjectString;
		FString ValueString;

		// Try to split value out
		if (!String.Split(TEXT("::"), &PackageString, &ValueString))
		{
			PackageString = String;
		}

		// Try to split on first . , if it fails PackageString will stay the same
		FString(PackageString).Split(TEXT("."), &PackageString, &ObjectString);

		return FAssetIdentifier(*PackageString, *ObjectString, *ValueString);
	}

	friend inline bool operator==(const FAssetIdentifier& A, const FAssetIdentifier& B)
	{
		return A.PackageName == B.PackageName && A.ObjectName == B.ObjectName && A.ValueName == B.ValueName;
	}

	friend inline uint32 GetTypeHash(const FAssetIdentifier& Key)
	{
		uint32 Hash = 0;

		// Most of the time only packagename is set
		if (Key.ObjectName.IsNone() && Key.ValueName.IsNone())
		{
			return GetTypeHash(Key.PackageName);
		}

		Hash = HashCombine(Hash, GetTypeHash(Key.PackageName));
		Hash = HashCombine(Hash, GetTypeHash(Key.ObjectName));
		Hash = HashCombine(Hash, GetTypeHash(Key.ValueName));
		return Hash;
	}
};

