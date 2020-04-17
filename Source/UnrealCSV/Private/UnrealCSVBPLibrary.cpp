// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "UnrealCSVBPLibrary.h"
#include "UnrealCSV.h"
#include "Engine/StreamableManager.h"

UUnrealCSVBPLibrary::UUnrealCSVBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

float UUnrealCSVBPLibrary::UnrealCSVSampleFunction(float Param)
{
	return -1;
}

void UUnrealCSVBPLibrary::GetCSVRowDataAndProperty(UDataTable* Table, FName InRowName, FName InColumnName, uint8*& OutRowData,UProperty*& OutProperty)
{
	if (	Table	==nullptr	)
	{
		return;
	}
	/*第一个生成属性数组*/
	for (	TFieldIterator	<UProperty>It (Table	->RowStruct	);It	;++It	)
	{
		UProperty* Prop = *It;
		check(Prop != NULL);

		//FName	PropName =FName	(*(Prop->GetName()));
		FString	PropString=Prop->GetName();
		FString	OutValue;
		FString	UnuseValue;
		PropString.Split("_", &OutValue,&UnuseValue);
		PropString = OutValue;
		FName	PropName =FName	(*PropString);

		if (!PropName.Compare(InColumnName	)	)
		{
			UProperty* ColumnProp = Prop;

			/*迭代row*/
			for (	auto	RowIt	=	Table->GetRowMap().CreateConstIterator()	;RowIt	;++RowIt	)
			{
				FName RowName = RowIt->Key;
				if (	!RowName	.Compare(InRowName	))
				{
					OutRowData = RowIt->Value;
					OutProperty = ColumnProp;
				}
			}
		}
	}
}

void UUnrealCSVBPLibrary::GetCSVTableString(UDataTable* Datatable, FString& OutResult)
{
	if (	!Datatable	->RowStruct	)
	{
		return;
	}

	/*表头部分,列*/
	OutResult = TEXT("---");
	FString Value_Out;
	FString	Value_Unuse;
	for (	TFieldIterator	<UProperty>It (Datatable->RowStruct	);It	;++It	)
	{
		UProperty* BaseProp = *It;
		check(BaseProp);

		OutResult += TEXT(",");
		OutResult += BaseProp->GetName();
		OutResult.Split("_", &Value_Out,&Value_Unuse);
		OutResult = Value_Out;
	}

	OutResult += TEXT("\n");

	/*Row，行部分*/
	for (auto RowIt = Datatable->GetRowMap().CreateConstIterator(); RowIt; ++RowIt)
	{
		FName RowName = RowIt->Key;
		OutResult += RowName.ToString();

		uint8* RowData = RowIt->Value;
		EDataTableExportFlags	 DTEF = EDataTableExportFlags::None;
		if (	!Datatable	->RowStruct	)
		{
			return;
		}
		for (	TFieldIterator	<UProperty	>It	(Datatable->RowStruct		);It	;++It	)
		{
			UProperty* BaseProp = *It;
			check(BaseProp);

			const void* Data = BaseProp->ContainerPtrToValuePtr<void	>(RowData, 0);
			OutResult += TEXT(",");

			const FString PropertyValue = DataTableUtils::GetPropertyValueAsString(BaseProp, RowData, DTEF);
			OutResult += TEXT("\"");
			OutResult += PropertyValue.Replace(TEXT("\""), TEXT("\"\""));
			OutResult += TEXT("\"");
		}
		OutResult += TEXT("\n");
	}
}

void UUnrealCSVBPLibrary::GetCSVRowNames(UDataTable* Table, TArray <FName>& OutRowNames)
{
	if (	Table	)
	{
		OutRowNames = Table->GetRowNames();
	}
	else
	{
		OutRowNames.Empty();
	}
}

int32 UUnrealCSVBPLibrary::GetIntValueFromDataTable(UDataTable* Table, FName InRowName, FName InColumnName)
{
	FString	ResultStr = GetStringValueFromDataTable(Table, InRowName, InColumnName);
	return	ResultStr.IsNumeric() ? FCString::Atoi(*ResultStr) : -1;
}

float UUnrealCSVBPLibrary::GetFloatValueFromDataTable(UDataTable* Table, FName InRowName, FName InColumnName)
{
	FString	ResultStr = GetStringValueFromDataTable(Table, InRowName, InColumnName);
	return	ResultStr.IsNumeric() ? FCString::Atof(*ResultStr) : -1.f;
}

FString UUnrealCSVBPLibrary::GetStringValueFromDataTable(UDataTable* Table, FName InRowName, FName InColumnName)
{
	uint8* Rowdata = nullptr;
	UProperty* ColumnProp = nullptr;

	/*Rowdate,ColumProp更新数据*/
	GetCSVRowDataAndProperty(Table, InRowName, InColumnName, Rowdata, ColumnProp);

	if (	Rowdata	!=nullptr	&&ColumnProp	!=nullptr	)
	{
		return	DataTableUtils::GetPropertyValueAsString(ColumnProp, Rowdata, EDataTableExportFlags::None);
	}
	return	TEXT("");
}

TAssetPtr<UTexture> UUnrealCSVBPLibrary::GetTextureFromDataTable(UDataTable* Table, FName InRowName, FName InColumnName)
{
	uint8* Rowdata = nullptr;
	UProperty* ColumnProp = nullptr;

	/*Rowdata,ColumProp更新数据*/
	GetCSVRowDataAndProperty(Table, InRowName, InColumnName, Rowdata, ColumnProp);
	
	if (Rowdata	!=nullptr	&&ColumnProp	!=nullptr	)
	{
		void* PropertyValue = ColumnProp->ContainerPtrToValuePtr<void	>(Rowdata, 0);
		FAssetPtr* AssetPtr = (FAssetPtr*)	PropertyValue	;

		FStringAssetReference	ID;
		UObject* Object = AssetPtr->Get();

		if (	Object	)
		{
			/*use object in case name has changed*/
			ID = FStringAssetReference(Object);
		}
		else
		{
			ID = AssetPtr->GetUniqueID();
		}

		/*Sync Load Assets*/
		FStreamableManager	SyncLoadManager;
		UTexture* TextureAsset = Cast<UTexture>(SyncLoadManager.LoadSynchronous(ID));
		TAssetPtr	<UTexture	>TextureAssetPtr = TextureAsset;
		return	TextureAssetPtr;
	}

	return	nullptr;
}

