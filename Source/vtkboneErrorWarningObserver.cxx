#include "vtkboneErrorWarningObserver.h"
#include <string>


//----------------------------------------------------------------------------
vtkboneErrorWarningObserver::vtkboneErrorWarningObserver()
:
  ErrorDescriptions(NULL),
  WarningDescriptions(NULL)
{
}

//----------------------------------------------------------------------------
vtkboneErrorWarningObserver::~vtkboneErrorWarningObserver()
{
  this->SetErrorDescriptions(NULL);
  this->SetWarningDescriptions(NULL);
}

//----------------------------------------------------------------------------
void vtkboneErrorWarningObserver::AppendErrorDescriptions(const char* newMsg)
{
  if (this->GetErrorDescriptions())
    {
    std::string s;
    s += this->GetErrorDescriptions();
    s += "\n";
    s += newMsg;
    this->SetErrorDescriptions(s.c_str());
    }
  else
    {
    this->SetErrorDescriptions(newMsg);
    }
}

//----------------------------------------------------------------------------
void vtkboneErrorWarningObserver::AppendWarningDescriptions(const char* newMsg)
{
  if (this->GetWarningDescriptions())
    {
    std::string s;
    s += this->GetWarningDescriptions();
    s += "\n";
    s += newMsg;
    this->SetWarningDescriptions(s.c_str());
    }
  else
    {
    this->SetWarningDescriptions(newMsg);
    }
}

//----------------------------------------------------------------------------
int vtkboneErrorWarningObserver::ErrorOccurred()
{
  return this->ErrorDescriptions != NULL;
}

//----------------------------------------------------------------------------
int vtkboneErrorWarningObserver::WarningOccurred()
{
  return this->WarningDescriptions != NULL;
}


//----------------------------------------------------------------------------
void vtkboneErrorWarningObserver::Reset()
{
  this->SetErrorDescriptions(NULL);
  this->SetWarningDescriptions(NULL);
}

//----------------------------------------------------------------------------
void vtkboneErrorWarningObserver::Execute
(
  vtkObject* caller,
  unsigned long eventId,
  void* callData
)
{
  // Block further error/warning logging once an error occurs.
  if (this->ErrorOccurred())
    { return; }
  const char* details = (const char*)callData;
  switch (eventId)
    {
    case vtkCommand::ErrorEvent:
      this->AppendErrorDescriptions(details);
      break;
    case vtkCommand::WarningEvent:
      this->AppendWarningDescriptions(details);
      break;
    }
}
