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

#ifndef PANE3D_H
#define PANE3D_H

#include <vtkPolyData.h>

#include "PointSelectionStyle3D.h"

#include <vtkSmartPointer.h>

#include <QVTKWidget.h>

struct Pane3D
{
  Pane3D(QVTKWidget* inputQVTKWidget);

  vtkSmartPointer<vtkRenderer> Renderer;

  PointSelectionStyle3D* SelectionStyle;

  QVTKWidget* qvtkWidget;

  void Refresh();
  
  vtkSmartPointer<vtkActor> PointCloudActor;
  vtkSmartPointer<vtkPolyDataMapper> PointCloudMapper;
};

#endif
