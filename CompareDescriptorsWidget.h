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

#ifndef CompareDescriptorsWidget_H
#define CompareDescriptorsWidget_H

#include "ui_CompareDescriptorsWidget.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkSeedWidget.h>

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>
#include <QFutureWatcher>
class QProgressDialog;

// Custom
#include "PointSelectionStyle3D.h"
#include "Types.h"

// Forward declarations
class vtkActor;
class vtkBorderWidget;
class vtkImageData;
class vtkImageActor;
class vtkPointPicker;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkRenderer;

class CompareDescriptorsWidget : public QMainWindow, public Ui::CompareDescriptorsWidget
{
  Q_OBJECT
public:

  // Constructor/Destructor
  CompareDescriptorsWidget();

  ~CompareDescriptorsWidget() {};

  void LoadPointCloud(const std::string& fileName);

  void ComputeDifferences();

public slots:
  void on_actionOpenPointCloud_activated();
  void on_btnCompute_clicked();

  void on_actionHelp_activated();
  void on_actionQuit_activated();

private:

  void PopulateArrayNames(vtkPolyData* const polyData);

  vtkSmartPointer<vtkPolyData> PointCloud;

  void SharedConstructor();
  QFutureWatcher<void> FutureWatcher;
  QProgressDialog* ProgressDialog;

  vtkSmartPointer<vtkPointPicker> PointPicker;

  vtkSmartPointer<vtkRenderer> Renderer;

  vtkSmartPointer<PointSelectionStyle3D> SelectionStyle;

  void Refresh();

  vtkSmartPointer<vtkActor> PointCloudActor;
  vtkSmartPointer<vtkPolyDataMapper> PointCloudMapper;

  vtkSmartPointer<vtkActor> MarkerActor;
  vtkSmartPointer<vtkPolyDataMapper> MarkerMapper;
  vtkSmartPointer<vtkSphereSource> MarkerSource;

  float MarkerRadius;

  void SelectedPointCallback(vtkObject* caller, long unsigned int eventId, void* callData);
};

#endif
