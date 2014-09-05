//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Appwizard generated plugin
// AUTHOR: 
//***************************************************************************/

#include "MaxExporter.h"
#include "Igame/igame.h"
#include "Igame/IGameModifier.h"
#include "bitmap.h"
#include "VertexIndex.h"
#include "VertexSet.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include "Vector3D.h"
#include "Vector2.h"
#include "TriangleBatch.h"
#include "NodeFace.h"
#include "loadSave.h"
#include "Node.h"


using namespace std;
using namespace gh;

#define MaxExporter_CLASS_ID	Class_ID(0xa741bd5e, 0x983f24c3)

class MaxExporter : public SceneExport {
	public:
		
		static HWND hParams;
		
		int				ExtCount();					// Number of extensions supported
		const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
		const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
		const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
		const TCHAR *	AuthorName();				// ASCII Author name
		const TCHAR *	CopyrightMessage();			// ASCII Copyright message
		const TCHAR *	OtherMessage1();			// Other message #1
		const TCHAR *	OtherMessage2();			// Other message #2
		unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
		void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box

		BOOL SupportsOptions(int ext, DWORD options);
		int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);
		void			findFaces( Node* myNode, ofstream& myFile );
		void			tearDown( IGameNode* gameNode );

		//Constructor/Destructor
		MaxExporter();
		~MaxExporter();
	private:

		std::set<IGameMaterial*> m_materialSet;
		//std::map< IGameNode*, std::map< IGameMaterial*, TriangleBatch* > > m_triangleBatchesPerNode;
		std::vector< Node* > m_NodeList;
		std::map< int, int > m_boneIDToNodeID;
		//std::map< IGameNode*, std::map<IGameMaterial*, std::vector< NodeFace > > > m_facesPerMaterialPerNode;
		loadSave BinaryFile;


};



class MaxExporterClassDesc : public ClassDesc2 
{
public:
	virtual int IsPublic() 							{ return TRUE; }
	virtual void* Create(BOOL /*loading = FALSE*/) 		{ return new MaxExporter(); }
	virtual const TCHAR *	ClassName() 			{ return GetString(IDS_CLASS_NAME); }
	virtual SClass_ID SuperClassID() 				{ return SCENE_EXPORT_CLASS_ID; }
	virtual Class_ID ClassID() 						{ return MaxExporter_CLASS_ID; }
	virtual const TCHAR* Category() 				{ return GetString(IDS_CATEGORY); }

	virtual const TCHAR* InternalName() 			{ return _T("MaxExporter"); }	// returns fixed parsable name (scripter-visible name)
	virtual HINSTANCE HInstance() 					{ return hInstance; }					// returns owning module handle
	

};


ClassDesc2* GetMaxExporterDesc() { 
	static MaxExporterClassDesc MaxExporterDesc;
	return &MaxExporterDesc; 
}





INT_PTR CALLBACK MaxExporterOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static MaxExporter *imp = NULL;

	switch(message) {
		case WM_INITDIALOG:
			imp = (MaxExporter *)lParam;
			CenterWindow(hWnd,GetParent(hWnd));
			return TRUE;

		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return 1;
	}
	return 0;
}


//--- MaxExporter -------------------------------------------------------
MaxExporter::MaxExporter()
{
}

MaxExporter::~MaxExporter() 
{

}

int MaxExporter::ExtCount()
{
	#pragma message(TODO("Returns the number of file name extensions supported by the plug-in."))
	return 1;
}

const TCHAR *MaxExporter::Ext(int n)
{		
	#pragma message(TODO("Return the 'i-th' file name extension (i.e. \"3DS\")."))
	return _T("GH");
}

const TCHAR *MaxExporter::LongDesc()
{
	#pragma message(TODO("Return long ASCII description (i.e. \"Targa 2.0 Image File\")"))
	return _T("Custom Guildhall 3DSMax File");
}
	
const TCHAR *MaxExporter::ShortDesc() 
{			
	#pragma message(TODO("Return short ASCII description (i.e. \"Targa\")"))
	return _T("Guildhall");
}

const TCHAR *MaxExporter::AuthorName()
{			
	#pragma message(TODO("Return ASCII Author name"))
	return _T("Timothy Scalzo");
}

const TCHAR *MaxExporter::CopyrightMessage() 
{	
	#pragma message(TODO("Return ASCII Copyright message"))
	return _T("");
}

const TCHAR *MaxExporter::OtherMessage1() 
{		
	//TODO: Return Other message #1 if any
	return _T("");
}

const TCHAR *MaxExporter::OtherMessage2() 
{		
	//TODO: Return other message #2 in any
	return _T("");
}

unsigned int MaxExporter::Version()
{				
	#pragma message(TODO("Return Version number * 100 (i.e. v3.01 = 301)"))
	return 100;
}

void MaxExporter::ShowAbout(HWND hWnd)
{			
	// Optional
}

BOOL MaxExporter::SupportsOptions(int ext, DWORD options)
{
	#pragma message(TODO("Decide which options to support.  Simply return true for each option supported by each Extension the exporter supports."))
	return TRUE;
}

void MaxExporter::findFaces( Node* myNode, ofstream& myFile )
{
	myFile << "NewNode\n";
	if( myNode != nullptr && myNode->m_gameNode != nullptr )
	{
		IGameObject* gameObject = myNode->m_gameNode->GetIGameObject();
		if( gameObject != nullptr )
		{
			m_boneIDToNodeID[ myNode->m_gameNode->GetNodeID() ] = myNode->m_id;
		
			
			//gameObject->InitializeData();
			if( gameObject->GetIGameType() == IGameObject::IGAME_MESH )
			{
				
				myFile << "Is a game mesh \n"; 
				IGameMesh* gameMesh = (IGameMesh*)gameObject;
				
				if( gameMesh != nullptr)
				{
					gameMesh->InitializeData();
					
					int numberOfFaces = gameMesh->GetNumberOfFaces();
					//std::map< IGameMaterial*, TriangleBatch* > triangleBatches;
					//std::map<IGameMaterial*, std::vector< NodeFace > > nodeFacePerMat;
					for( int i = 0; i < numberOfFaces; ++i )
					{
						FaceEx* meshFace = gameMesh->GetFace( i );
						NodeFace nodeFace( gameMesh, meshFace );
						IGameMaterial* gameMaterial = gameMesh->GetMaterialFromFace( meshFace );

						m_materialSet.insert( gameMaterial );
						auto found = myNode->m_triangleBatchesPerMaterial.find( gameMaterial );
						if( found == myNode->m_triangleBatchesPerMaterial.end() )
						{
							MaxMaterial* material = new MaxMaterial( gameMaterial );
							myNode->m_triangleBatchesPerMaterial[ gameMaterial ] = new TriangleBatch( material, new VBO(), new IBO() );
							std::vector< NodeFace > faces;
							faces.push_back(nodeFace);
							myNode->m_facesPerMaterial[ gameMaterial ] = faces;
						}
						else
						{
							auto findFace = myNode->m_facesPerMaterial.find( gameMaterial );
							findFace->second.push_back( nodeFace );
						}

					}
					//myNode->addNodeFaces( nodeFacePerMat );
					//m_triangleBatchesPerNode[gameNode] = triangleBatches;
				}
				
			}
			

		}
		
		int childCount = myNode->m_gameNode->GetChildCount();
		myFile<< childCount << "\n";
		
		for( int j = 0; j < childCount; ++j )
		{

			IGameNode* childGameNode = myNode->m_gameNode->GetNodeChild( j );
			if( childGameNode != nullptr )
			{
				Node* childNode = new Node( childGameNode );
				childNode->setParent( myNode );
				m_NodeList.push_back( childNode );
				findFaces( childNode, myFile );
			}

		}
		//gameNode->ReleaseIGameObject();
	}
}

void MaxExporter::tearDown( IGameNode* gameNode )
{
	int childCount = gameNode->GetChildCount();
	for( int j = 0; j < childCount; ++j )
	{
		IGameNode* childGameNode = gameNode->GetNodeChild( j );
		tearDown( childGameNode );
	}
	gameNode->ReleaseIGameObject();
}

int	MaxExporter::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options)
{

	/*if(!suppressPrompts)
		DialogBoxParam(hInstance, 
				MAKEINTRESOURCE(IDD_PANEL), 
				GetActiveWindow(), 
				MaxExporterOptionsDlgProc, (LPARAM)this);*/

	#pragma message(TODO("return TRUE If the file is exported properly"))

	Node::s_nextID = 1;
	ofstream myFile;
	myFile.open("DebugExporter.txt");
	wstring wFileName( name );
	string fileName( wFileName.begin(), wFileName.end() );
	//f= fopen( fileName.c_str(),"wb");
	BinaryFile = loadSave( fileName.c_str() );
	//BinaryFile.saveInt( 5 );
	//BinaryFile.close();
	myFile << "Start Export\n";

	IGameScene* gameScene = GetIGameInterface();
	gameScene->InitialiseIGame();
	int nodeCount = gameScene->GetTopLevelNodeCount();
	myFile << "Number of top level nodes: " << nodeCount << "\n";
	//get all of the materials
	for( int nodeNumber = 0; nodeNumber < nodeCount; ++nodeNumber)
	{
		IGameNode* gameNode = gameScene->GetTopLevelNode( nodeNumber );
		
		Node* myNode = new Node( gameNode );
		m_NodeList.push_back( myNode );
		findFaces( myNode, myFile );
		
	}

	myFile << "Number of materials\n";
	myFile << m_materialSet.size() << "\n";
	

	int totalBatches = 0;
	//myFile<< "Number of triangleBatchMaps: " << m_triangleBatchesPerNode.size() << "\n";
	for( auto nodeIter = m_NodeList.begin(); nodeIter != m_NodeList.end(); ++nodeIter )
	{
		std::map< IGameMaterial*, TriangleBatch* > triangleBatches = (*nodeIter)->m_triangleBatchesPerMaterial;
		myFile << "Invidiual triangleBatch size: " << triangleBatches.size() << "\n";

		for( auto materialIter = m_materialSet.begin(); materialIter != m_materialSet.end(); ++materialIter )
		{
			auto found = triangleBatches.find( * materialIter );
			if( found != triangleBatches.end() )
			{
				++totalBatches;
			}
		}
	}

	BinaryFile.saveInt( m_NodeList.size() );
	myFile << "Number of Nodes: " << m_NodeList.size() << "\n";
	
	for( auto nodeIter = m_NodeList.begin(); nodeIter != m_NodeList.end(); ++nodeIter )
	{
		std::map< IGameMaterial*, TriangleBatch* > triangleBatches = (*nodeIter)->m_triangleBatchesPerMaterial;
		std::map<IGameMaterial*, std::vector< NodeFace > > facesPerMaterial = (*nodeIter)->m_facesPerMaterial;
		IGameNode* currentNode = (*nodeIter)->m_gameNode;
		IGameNode* parentNode;
		GMatrix parentWTM;
		GMatrix toParentMatrix;
		GMatrix worldTM;
		GMatrix localTM;
		int time = gameScene->GetSceneStartTime();
		for( ; time < gameScene->GetSceneEndTime(); time += 4800/30 )
		{
			if( (*nodeIter)->m_parentID != 0 )
			{
				myFile << "Trying to find parent... \n";
				parentNode = (*nodeIter)->m_parent->m_gameNode;
				if( parentNode != nullptr )
				{
					myFile << "Parent found \n";
					parentWTM = parentNode->GetWorldTM( time );
					toParentMatrix = parentWTM.Inverse();

					worldTM = currentNode->GetWorldTM(  time ) * toParentMatrix;

				}

			}
			else
			{
				worldTM = currentNode->GetWorldTM( time );
				
			}
			(*nodeIter)->m_toParentMatrix.push_back( Matrix4x4( worldTM[0], worldTM[1], worldTM[2], worldTM[3] ) );
		}


		localTM = currentNode->GetWorldTM().Inverse();
		(*nodeIter)->m_worldToLocal = Matrix4x4(  localTM[0], localTM[1], localTM[2], localTM[3] );
		

		//Save the node
		BinaryFile.saveNode( *nodeIter, myFile );
		BinaryFile.saveInt( (*nodeIter)->m_triangleBatchesPerMaterial.size() );
	
		for( auto materialIter = m_materialSet.begin(); materialIter != m_materialSet.end(); ++materialIter )
		{
			//Set the current material's VBO and IBO
			//
			IGameMaterial* currentMaterial = *materialIter;
			TriangleBatch* currentBatch = triangleBatches[ currentMaterial ];
			
			GMatrix localTMNoTrans = localTM;
			localTMNoTrans.SetRow( 3, Point4( 0,0,0,1) );
			if( currentBatch != nullptr )
			{
				
				MaxMaterial* currentMaxMaterial = currentBatch->m_material;
				VBO* currentVBO = currentBatch->m_vbo;
				IBO* currentIBO = currentBatch->m_ibo;
				vector< NodeFace >& faceVector = facesPerMaterial.find( currentMaterial )->second;

				//Get texture materials and export them
				//
				if( currentMaterial != nullptr )
				{
					int numOfTexMaps = currentMaterial->GetNumberOfTextureMaps();
					myFile << "Number of texture maps: " << numOfTexMaps << "\n";
					for( int i = 0; i < numOfTexMaps; ++i )
					{
						IGameTextureMap* gameTextureMap = currentMaterial->GetIGameTextureMap( i );
						
						if( gameTextureMap != nullptr && gameTextureMap->IsEntitySupported() )
						{
							int stdMapSlot = gameTextureMap->GetStdMapSlot();
							if( stdMapSlot == ID_DI )
							{
								wstring wBitmapFileName;
								wBitmapFileName = gameTextureMap->GetBitmapFileName();
								if( wBitmapFileName.size() > 0 )
								{
									BitmapInfo bi( gameTextureMap->GetBitmapFileName() );
									BMMGetFullFilename( &bi );
									wBitmapFileName = bi.Name();
									std::string fullBitmapFileName( wBitmapFileName.begin(), wBitmapFileName.end() );
									if( fullBitmapFileName.size() > 0 )
									{
										int lastSlash = fullBitmapFileName.find_last_of('\\') + 1;
										if( lastSlash != string::npos )
										{
											const std::string bitmapFileName = fullBitmapFileName.substr( lastSlash );
											wstring nameAsWString( name );
											std::string nameAsString( nameAsWString.begin(), nameAsWString.end() );
											const std::string extension = nameAsString.substr( 0, nameAsString.find_last_of('\\') + 1 );
											std::string newFileName = extension;
											newFileName.append( bitmapFileName );
											wstring wNewFileName(newFileName.begin(), newFileName.end());
											if( CopyFile( wBitmapFileName.c_str(), wNewFileName.c_str(), false ) )
											{
												if( stdMapSlot == ID_DI )
												{
													currentMaxMaterial->m_diffuseTexture = bitmapFileName;
													currentMaxMaterial->bHasDiffuseTexture = true;
												}
											//BinaryFile.saveString( bitmapFileName );
											}
											else
											{
												myFile << GetLastError() << "\n";
												myFile << "copying the file FAILED.\n";
											}
										}
									}	
								}
							}
						}
					}
				}
				
				myFile<< "Number of faces for this material: " << faceVector.size() << "\n";
				for( int face = 0; face < faceVector.size(); ++face )
				{
					FaceEx* meshFace = faceVector[face].m_face;
					IGameMesh* gameMesh = faceVector[face].m_mesh;
					IGameSkin* gameSkin = gameMesh->GetIGameSkin();
					int position, normal, color, texCoordinate, maxPosition;
					for( int i = 0; i < 3; ++i)
					{
						
						maxPosition = (int)meshFace->vert[i];
						
						Point3 tempPos = gameMesh->GetVertex( maxPosition );
						tempPos = tempPos * localTM;
						Vector3D positionVec3( tempPos.x, tempPos.y, tempPos.z );
						position = currentVBO->insertPosition(positionVec3);

						normal = (int)meshFace->norm[i];
						Point3 tempNormal = gameMesh->GetNormal( normal );
						tempNormal = tempNormal * localTMNoTrans;
						tempNormal = tempNormal.Normalize();
						
						Vector3D normalVec3( tempNormal.x, tempNormal.y, tempNormal.z );
						normal = currentVBO->insertNormal(normalVec3);

						//IBO
						texCoordinate = (int)meshFace->texCoord[i];
						Point2 tempTexCoord = gameMesh->GetTexVertex( texCoordinate );
						Vector2 texCoordVec2( tempTexCoord.x, tempTexCoord.y );
						texCoordinate = currentVBO->insertTexCoord(texCoordVec2);
						VertexIndex VI( position, normal, texCoordinate );
						if( gameSkin != nullptr )
						{
							int numberOfBones = gameSkin->GetNumberOfBones( maxPosition );
							for( int boneIndex = 0; boneIndex < numberOfBones; ++boneIndex )
							{
								float boneWeight = gameSkin->GetWeight( maxPosition, boneIndex );
								IGameNode* bone = gameSkin->GetIGameBone( maxPosition, boneIndex );
								myFile << "Bone node ID: " << bone->GetNodeID() << "\n";
								int nodeIDForBone = m_boneIDToNodeID[ bone->GetNodeID() ];
								myFile << "Node ID: " << nodeIDForBone << "\n";
								VI.addBoneWeight( nodeIDForBone, boneWeight );

								/*for( auto boneIter = m_NodeList.begin(); boneIter != m_NodeList.end(); ++boneIter )
								{
									if( (*boneIter)->m_gameNode == bone )
									{
										myFile << "Found the bone!\n"; 
										
									}
								}*/
								
							}
							VI.topBoneWeights();
						}

						int vertIndex = currentVBO->insertVertex( VI );
						currentIBO->addIndex( vertIndex );

					}

				}
				
				BinaryFile.saveTriangleMesh( currentBatch, myFile );
			}
		}

	}
	myFile.close();

	for( int nodeNumber = 0; nodeNumber < nodeCount; ++nodeNumber)
	{
		IGameNode* gameNode = gameScene->GetTopLevelNode( nodeNumber );
		if( gameNode != nullptr )
		{
			tearDown( gameNode );
		}
		
	}

	BinaryFile.close();
	return TRUE;

	//return FALSE;
}


