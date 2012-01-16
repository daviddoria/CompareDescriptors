/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "PointSelectionStyle3D.h"

// VTK
#include <vtkAbstractPicker.h>
#include <vtkCamera.h>
#include <vtkObjectFactory.h>
#include <vtkPointPicker.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>

// STL
#include <sstream>

// Custom
#include "Helpers.h"

vtkStandardNewMacro(PointSelectionStyle3D);

PointSelectionStyle3D::PointSelectionStyle3D()
{
  this->MarkerRadius = .05;
  
  // Create a sphere to use as the dot
  this->MarkerSource = vtkSmartPointer<vtkSphereSource>::New();
  this->MarkerSource->SetRadius(this->MarkerRadius);
  this->MarkerSource->Update();

  this->MarkerMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->MarkerMapper->SetInputConnection(this->MarkerSource->GetOutputPort());

  this->MarkerActor = vtkSmartPointer<vtkActor>::New();
  this->MarkerActor->SetMapper(this->MarkerMapper);

  vtkSmartPointer<vtkActor> MarkerActor;
  vtkSmartPointer<vtkPolyDataMapper> MarkerMapper;
  vtkSmartPointer<vtkSphereSource> MarkerSource;
}

void PointSelectionStyle3D::SetMarkerRadius(const float radius)
{
  this->MarkerSource->SetRadius(radius);
}

void PointSelectionStyle3D::OnLeftButtonDown() 
{
  vtkPointPicker::SafeDownCast(this->Interactor->GetPicker())->Pick(this->Interactor->GetEventPosition()[0],
          this->Interactor->GetEventPosition()[1],
          0,  // always zero.
          this->CurrentRenderer);

  double picked[3] = {0,0,0};

  vtkPointPicker::SafeDownCast(this->Interactor->GetPicker())->GetPickPosition(picked);
  this->SelectedPointId = vtkPointPicker::SafeDownCast(this->Interactor->GetPicker())->GetPointId();
  //std::cout << "Picked point with coordinate: " << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;

  if(this->Interactor->GetShiftKey())
    {
    this->CurrentRenderer->GetActiveCamera()->SetFocalPoint(picked);
    }

  // Only select the point if control is held
  if(this->Interactor->GetControlKey())
    {
    ChangePoint(this->SelectedPointId);
    }

  // Forward events
  vtkInteractorStyleTrackballCamera::OnLeftButtonDown();

}

double* PointSelectionStyle3D::GetMarkerLocation()
{
  return this->MarkerSource->GetCenter();
}

void PointSelectionStyle3D::ChangePoint(const vtkIdType pointId)
{
  double p[3];
  this->Points->GetPoint(pointId, p);
  std::cout << "Changed point to: " << p[0] << " " << p[1] << " " << p[2] << std::endl;
  this->MarkerSource->SetCenter(p);
}

void PointSelectionStyle3D::SetCurrentRenderer(vtkRenderer* const renderer)
{
  vtkInteractorStyle::SetCurrentRenderer(renderer);

  renderer->AddViewProp( this->MarkerActor );
}
