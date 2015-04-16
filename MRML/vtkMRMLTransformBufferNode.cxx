

#include "vtkMRMLTransformBufferNode.h"



// Constants ------------------------------------------------------------------
static const char* ACTIVE_TRANSFORM_REFERENCE_ROLE = "ActiveTransform";



// Standard MRML Node Methods ------------------------------------------------------------

vtkMRMLTransformBufferNode* vtkMRMLTransformBufferNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLTransformBufferNode" );
  if( ret )
    {
      return ( vtkMRMLTransformBufferNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTransformBufferNode();
}


vtkMRMLNode* vtkMRMLTransformBufferNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLTransformBufferNode" );
  if( ret )
    {
      return ( vtkMRMLTransformBufferNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTransformBufferNode();
}



void vtkMRMLTransformBufferNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


void vtkMRMLTransformBufferNode
::WriteXML( ostream& of, int nIndent )
{
  this->Superclass::WriteXML(of, nIndent);
}


void vtkMRMLTransformBufferNode
::ReadXMLAttributes( const char** atts )
{
  this->Superclass::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);

    // do something...
  }

}


void vtkMRMLTransformBufferNode
::Copy( vtkMRMLNode* anode )
{
  this->Superclass::Copy( anode );
  vtkMRMLTransformBufferNode* node = ( vtkMRMLTransformBufferNode* ) anode;

  this->Clear();

  // Transform buffers
  std::map< std::string, vtkSmartPointer< vtkLogRecordBuffer > >::iterator itr;
  for( itr = node->TransformRecordBuffers.begin(); itr != node->TransformRecordBuffers.end(); itr++ )
  {
    vtkSmartPointer< vtkLogRecordBuffer > newTransformRecordBuffer = vtkSmartPointer< vtkLogRecordBuffer >::New();
    newTransformRecordBuffer->Copy( itr->second );
    std::string newTransformRecordBufferName = vtkTransformRecord::SafeDownCast( newTransformRecordBuffer->GetCurrentRecord() )->GetDeviceName(); // TODO: Make more robust
    this->TransformRecordBuffers[ newTransformRecordBufferName ] = newTransformRecordBuffer;
  }

  // Message buffer
  vtkSmartPointer< vtkLogRecordBuffer > newMessageRecordBuffer = vtkSmartPointer< vtkLogRecordBuffer >::New();
  newMessageRecordBuffer->Copy( node->MessageRecordBuffer );
  this->MessageRecordBuffer = newMessageRecordBuffer;

  // Recording state
  this->RecordingState = node->RecordingState;
}



// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLTransformBufferNode
::vtkMRMLTransformBufferNode()
{
  // No need to initialize the vectors
  this->MessageRecordBuffer = vtkSmartPointer< vtkLogRecordBuffer >::New();

  this->AddNodeReferenceRole( ACTIVE_TRANSFORM_REFERENCE_ROLE );

  this->RecordingState = false;
  this->Clock0 = clock();
}


vtkMRMLTransformBufferNode
::~vtkMRMLTransformBufferNode()
{
  this->Clear();
}



// Transforms and Messages ------------------------------------------------------------------------


void vtkMRMLTransformBufferNode
::Concatenate( vtkMRMLTransformBufferNode* catBuffer )
{
  // Note: Does not affect the active transforms or recording state

  // If you want things to be deep copied, just deep copy the buffer
  vtkSmartPointer< vtkLogRecordBuffer > otherCombinedTransformRecordBuffer = vtkSmartPointer< vtkLogRecordBuffer >::New();
  catBuffer->GetCombinedTransformRecordBuffer( otherCombinedTransformRecordBuffer ); // This will maintain complexity
  for ( int i = 0; i < otherCombinedTransformRecordBuffer->GetNumRecords(); i++ )
  {
    this->AddTransform( vtkTransformRecord::SafeDownCast( otherCombinedTransformRecordBuffer->GetRecord( i ) ) );
  }

  for ( int i = 0; i < catBuffer->GetNumMessages(); i++ )
  {
    this->AddMessage( catBuffer->GetMessage( i ) );
  }

}


void vtkMRMLTransformBufferNode
::AddTransform( vtkTransformRecord* newTransform )
{
  // If the relevant transform record buffer does not exist, then create it
  if ( this->TransformRecordBuffers.find( newTransform->GetDeviceName() ) == this->TransformRecordBuffers.end() )
  {
    vtkSmartPointer< vtkLogRecordBuffer > newTransformRecordBuffer = vtkSmartPointer< vtkLogRecordBuffer >::New();
    this->TransformRecordBuffers[ newTransform->GetDeviceName() ] = newTransformRecordBuffer;
  }

  // Add to the appropriate transform record buffer
  int insertLocation = this->TransformRecordBuffers[ newTransform->GetDeviceName() ]->AddRecord( newTransform );

  // Invoke appropriate events
  this->Modified();
  this->InvokeEvent( this->TransformAddedEvent, &insertLocation );
}


void vtkMRMLTransformBufferNode
::AddMessage( vtkMessageRecord* newMessage )
{
  // Add to the message record buffer
  int insertLocation = this->MessageRecordBuffer->AddRecord( newMessage );

  // Invoke appropriate events
  this->Modified();
  this->InvokeEvent( this->MessageAddedEvent, &insertLocation );
}


void vtkMRMLTransformBufferNode
::RemoveTransform( int index, std::string transformName )
{
  // If there is no such transform record buffer, then do nothing
  if ( this->TransformRecordBuffers.find( transformName ) == this->TransformRecordBuffers.end() )
  {
    return;
  }

  // Only invoke events if the remove was successful
  if ( this->TransformRecordBuffers[ transformName ]->RemoveRecord( index ) )
  {
    this->Modified();
    this->InvokeEvent( this->TransformRemovedEvent );
  }
}


void vtkMRMLTransformBufferNode
::RemoveTransformsByName( std::string name )
{
  // If there is no such transform record buffer, then do nothing
  if ( this->TransformRecordBuffers.find( name ) == this->TransformRecordBuffers.end() )
  {
    return;
  }

  this->TransformRecordBuffers.erase( name );
  this->Modified();
  this->InvokeEvent( this->TransformRemovedEvent );
}


void vtkMRMLTransformBufferNode
::RemoveMessage( int index )
{
  // Only invoke events if the remove was successful
  if ( this->MessageRecordBuffer->RemoveRecord( index ) )
  {
    this->Modified();
    this->InvokeEvent( this->MessageRemovedEvent );
  }
}


void vtkMRMLTransformBufferNode
::RemoveMessagesByName( std::string name )
{
  for ( int i = 0; i < this->MessageRecordBuffer->GetNumRecords(); i++ )
  {
    if ( this->GetMessage(i)->GetMessageString().compare( name ) == 0 )
	  {
      this->RemoveMessage( i );
	    i--;
	  }
  }

  this->Modified();
  this->InvokeEvent( this->MessageRemovedEvent );
}


vtkTransformRecord* vtkMRMLTransformBufferNode
::GetTransform( int index, std::string transformName )
{
  if ( this->TransformRecordBuffers.find( transformName ) == this->TransformRecordBuffers.end() )
  {
    return NULL;
  }

  return vtkTransformRecord::SafeDownCast( this->TransformRecordBuffers[ transformName ]->GetRecord( index ) );
}


vtkTransformRecord* vtkMRMLTransformBufferNode
::GetCurrentTransform( std::string transformName )
{
  if ( this->TransformRecordBuffers.find( transformName ) == this->TransformRecordBuffers.end() )
  {
    return NULL;
  }

  return vtkTransformRecord::SafeDownCast( this->TransformRecordBuffers[ transformName ]->GetCurrentRecord() );
}


vtkMessageRecord* vtkMRMLTransformBufferNode
::GetMessage( int index )
{
  return vtkMessageRecord::SafeDownCast( this->MessageRecordBuffer->GetRecord( index ) );
}


vtkMessageRecord* vtkMRMLTransformBufferNode
::GetCurrentMessage()
{
  return vtkMessageRecord::SafeDownCast( this->MessageRecordBuffer->GetCurrentRecord() );
}


vtkTransformRecord* vtkMRMLTransformBufferNode
::GetTransformAtTime( double time, std::string transformName )
{
  if ( this->TransformRecordBuffers.find( transformName ) == this->TransformRecordBuffers.end() )
  {
    return NULL;
  }

  return vtkTransformRecord::SafeDownCast( this->TransformRecordBuffers[ transformName ]->GetRecordAtTime( time ) );
}


vtkMessageRecord* vtkMRMLTransformBufferNode
::GetMessageAtTime( double time )
{
  return vtkMessageRecord::SafeDownCast( this->MessageRecordBuffer->GetRecordAtTime( time ) );
}


std::vector< std::string > vtkMRMLTransformBufferNode
::GetAllRecordedTransforms()
{
  std::vector< std::string > recordedTransforms;
  std::map< std::string, vtkSmartPointer< vtkLogRecordBuffer > >::iterator itr;
  for( itr = this->TransformRecordBuffers.begin(); itr != this->TransformRecordBuffers.end(); itr++ )
  {
    recordedTransforms.push_back( itr->first );
  }

  return recordedTransforms;
}



int vtkMRMLTransformBufferNode
::GetNumTransforms()
{
  // Add over all transform names
  int totalTransforms = 0;
  std::map< std::string, vtkSmartPointer< vtkLogRecordBuffer > >::iterator itr;
  for( itr = this->TransformRecordBuffers.begin(); itr != this->TransformRecordBuffers.end(); itr++ )
  {
    totalTransforms += itr->second->GetNumRecords();
  }

  return totalTransforms;
}


int vtkMRMLTransformBufferNode
::GetNumMessages()
{
  return this->MessageRecordBuffer->GetNumRecords();
}


// Report the maximum transform time - use only the message time if no transforms exist
double vtkMRMLTransformBufferNode
::GetMaximumTime()
{
  // Max over all transform names
  int maxTime = - std::numeric_limits< double >::max();
  std::map< std::string, vtkSmartPointer< vtkLogRecordBuffer > >::iterator itr;
  for( itr = this->TransformRecordBuffers.begin(); itr != this->TransformRecordBuffers.end(); itr++ )
  {
    if ( itr->second->GetMaximumTime() > maxTime )
    {
      maxTime = itr->second->GetMaximumTime();
    }
  }

  if ( this->MessageRecordBuffer->GetMaximumTime() > maxTime )
  {
    maxTime = this->MessageRecordBuffer->GetMaximumTime();
  }

  return 0.0;
}


// Report the minimum transform time - use only the message time if no transforms exist
double vtkMRMLTransformBufferNode
::GetMinimumTime()
{
  // Min over all transform names
  int minTime = std::numeric_limits< double >::max();
  std::map< std::string, vtkSmartPointer< vtkLogRecordBuffer > >::iterator itr;
  for( itr = this->TransformRecordBuffers.begin(); itr != this->TransformRecordBuffers.end(); itr++ )
  {
    if ( itr->second->GetMinimumTime() < minTime )
    {
      minTime = itr->second->GetMinimumTime();
    }
  }

  if ( this->MessageRecordBuffer->GetMinimumTime() < minTime )
  {
    minTime = this->MessageRecordBuffer->GetMinimumTime();
  }

  return 0.0;
}


double vtkMRMLTransformBufferNode
::GetTotalTime()
{
  return this->GetMaximumTime() - this->GetMinimumTime();
}


void vtkMRMLTransformBufferNode
::Clear()
{
  this->ClearTransforms();
  this->ClearMessages();
}


void vtkMRMLTransformBufferNode
::ClearTransforms()
{
  // No need to explicitly call the VTK delete function on each of these VTK objects (since we use smart pointers)
  this->TransformRecordBuffers.clear(); // Everything else will be automatically taken care of by smart pointers
}


void vtkMRMLTransformBufferNode
::ClearMessages()
{
  // Do not delete the buffer, just clear it
  this->MessageRecordBuffer->Clear();
}


// Combine all of the transform record buffers into one big record buffer with all transforms in it
void vtkMRMLTransformBufferNode
::GetCombinedTransformRecordBuffer( vtkLogRecordBuffer* combinedTransformRecordBuffer )
{
  // TODO: Improve the complexity (currently it is O( nlogn ), but with a merge type method, we can make it O( n )).
  // Iterate over all transform record buffers
  std::map< std::string, vtkSmartPointer< vtkLogRecordBuffer > >::iterator itr;
  for( itr = this->TransformRecordBuffers.begin(); itr != this->TransformRecordBuffers.end(); itr++ )
  {
    combinedTransformRecordBuffer->Concatenate( itr->second );
  }  
}





// Active transforms -------------------------------------------------------------------------

void vtkMRMLTransformBufferNode
::AddActiveTransformID( std::string transformID )
{
  vtkNew< vtkIntArray > events;
  events->InsertNextValue( vtkMRMLLinearTransformNode::TransformModifiedEvent );
  this->AddAndObserveNodeReferenceID( ACTIVE_TRANSFORM_REFERENCE_ROLE, transformID.c_str(), events.GetPointer() );

  this->InvokeEvent( this->ActiveTransformAddedEvent );
}


void vtkMRMLTransformBufferNode
::RemoveActiveTransformID( std::string transformID )
{
  // Check all referenced node IDs
  for ( int i = 0; i < this->GetNumberOfNodeReferences( ACTIVE_TRANSFORM_REFERENCE_ROLE ); i++ )
  {
    if ( transformID.compare( this->GetNthNodeReferenceID( ACTIVE_TRANSFORM_REFERENCE_ROLE, i ) ) == 0 )
    {
      this->RemoveNthNodeReferenceID( ACTIVE_TRANSFORM_REFERENCE_ROLE, i );
      this->InvokeEvent( this->ActiveTransformRemovedEvent );
	    i--;      
	  }
  }
}


std::vector< std::string > vtkMRMLTransformBufferNode
::GetActiveTransformIDs()
{
  std::vector< std::string > transformIDs;

  // Check all referenced node IDs
  for ( int i = 0; i < this->GetNumberOfNodeReferences( ACTIVE_TRANSFORM_REFERENCE_ROLE ); i++ )
  {
    transformIDs.push_back( this->GetNthNodeReferenceID( ACTIVE_TRANSFORM_REFERENCE_ROLE, i ) );
  }

  return transformIDs;
}
  

void vtkMRMLTransformBufferNode
::SetActiveTransformIDs( std::vector< std::string > transformIDs )
{
  // Remove all of the active transform IDs
  while( this->GetNumberOfNodeReferences( ACTIVE_TRANSFORM_REFERENCE_ROLE ) > 0 )
  {
    this->RemoveNthNodeReferenceID( ACTIVE_TRANSFORM_REFERENCE_ROLE, 0 );
  }

  // Add all of the specified IDs
  for ( int i = 0; i < transformIDs.size(); i++ )
  {
    this->AddActiveTransformID( transformIDs.at( i ) );
  }
}



// Recording ---------------------------------------------------------------------------------

void vtkMRMLTransformBufferNode
::StartRecording()
{
  this->RecordingState = true;
}


void vtkMRMLTransformBufferNode
::StopRecording()
{
  this->RecordingState = false;
}


double vtkMRMLTransformBufferNode
::GetCurrentTimestamp()
{
  clock_t clock1 = clock();  
  return double( clock1 - this->Clock0 ) / CLOCKS_PER_SEC;
}



// MRML node event processing -----------------------------------------------------------------

void vtkMRMLTransformBufferNode
::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  // Do nothing if the node is not recording
  if ( ! this->RecordingState )
  {
    return;
  }

  // The caller will be the node that was modified
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( caller );
  if ( transformNode == NULL )
  {
    return;
  }

  vtkSmartPointer< vtkMatrix4x4 > transformMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  transformNode->GetMatrixTransformToParent( transformMatrix );

  // Record the transform into a string  
  std::stringstream matrixsstring;
  for ( int row = 0; row < 4; ++ row )
  {
    for ( int col = 0; col < 4; ++ col )
    {
      matrixsstring << transformMatrix->GetElement( row, col ) << " ";
    }
  }

  // Find the relevant record buffer, and make sure it is not a duplicate
  if ( this->TransformRecordBuffers.find( transformNode->GetName() ) == this->TransformRecordBuffers.end() )
  {
    return;
  }
  
  // Look for the most recent value of this transform
  // If the value hasn't changed, we don't record
  vtkTransformRecord* testDuplicateRecord = vtkTransformRecord::SafeDownCast( this->TransformRecordBuffers[ transformNode->GetName() ]->GetCurrentRecord() );
  if ( testDuplicateRecord->GetTransformString().compare( matrixsstring.str() ) == 0 )
  {
    return; // If it is a duplicate then exit, we have nothing to record  
  }


  vtkSmartPointer< vtkTransformRecord > newTransformRecord = vtkSmartPointer< vtkTransformRecord >::New();
  newTransformRecord->SetTransformString( matrixsstring.str() );
  newTransformRecord->SetDeviceName( transformNode->GetName() );
  newTransformRecord->SetTime( this->GetCurrentTimestamp() );
  this->AddTransform( newTransformRecord );

}


// Read/write XML files ------------------------------------------------------------------------

std::string vtkMRMLTransformBufferNode
::ToXMLString()
{
  std::stringstream xmlstring;

  vtkSmartPointer< vtkLogRecordBuffer > combinedTransformRecordBuffer = vtkSmartPointer< vtkLogRecordBuffer >::New();
  this->GetCombinedTransformRecordBuffer( combinedTransformRecordBuffer ); // This will maintain complexity
  
  xmlstring << "<TransformRecorderLog>" << std::endl;

  for ( int i = 0; i < combinedTransformRecordBuffer->GetNumRecords(); i++ )
  {
    xmlstring << combinedTransformRecordBuffer->GetRecord(i)->ToXMLString();
  }

  for ( int i = 0; i < this->MessageRecordBuffer->GetNumRecords(); i++ )
  {
    xmlstring << this->MessageRecordBuffer->GetRecord(i)->ToXMLString();
  }

  xmlstring << "</TransformRecorderLog>" << std::endl;

  return xmlstring.str();
}


void vtkMRMLTransformBufferNode
::FromXMLElement( vtkXMLDataElement* rootElement )
{
  if ( ! rootElement || strcmp( rootElement->GetName(), "TransformRecorderLog" ) != 0 ) 
  {
    return;
  }

  int numElements = rootElement->GetNumberOfNestedElements();  // Number of saved records (including transforms and messages).
  
  for ( int i = 0; i < numElements; i++ )
  {
    vtkXMLDataElement* element = rootElement->GetNestedElement( i );

	  if ( element == NULL || strcmp( element->GetName(), "log" ) != 0 )
	  {
      continue;
	  }

	  if ( strcmp( element->GetAttribute( "type" ), "transform" ) == 0 )
    {
		  vtkSmartPointer< vtkTransformRecord > newTransform = vtkSmartPointer< vtkTransformRecord >::New();
		  newTransform->FromXMLElement( element );
		  this->AddTransform( newTransform );
		  continue;
    }

    if ( strcmp( element->GetAttribute( "type" ), "message" ) == 0 )
    {
		  vtkSmartPointer< vtkMessageRecord > newMessage = vtkSmartPointer< vtkMessageRecord >::New();
		  newMessage->FromXMLElement( element );
		  this->AddMessage( newMessage );
		  continue;
    }
   
  }

  // Status updates are taken care of in the add functions
}
