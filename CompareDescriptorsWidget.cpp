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

#include "ui_CompareDescriptorsWidget.h"
#include "CompareDescriptorsWidget.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkVector.h"

// Qt
#include <QFileDialog>
#include <QIcon>
#include <QProgressDialog>
#include <QTextEdit>
#include <QtConcurrentRun>

// VTK
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkFloatArray.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPointPicker.h>
#include <vtkProperty2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkImageSliceMapper.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>

// Custom
#include "Helpers.h"
#include "Types.h"
#include "PointSelectionStyle3D.h"
#include "Pane3D.h"

void CompareDescriptorsWidget::on_actionHelp_activated()
{
  QTextEdit* help=new QTextEdit();

  help->setReadOnly(true);
  help->append("<h1>Image correspondences</h1>\
  Hold the right mouse button and drag to zoom in and out. <br/>\
  Hold the middle mouse button and drag to pan the image. <br/>\
  Click the left mouse button to select a keypoint.<br/> <p/>\
  <h1>Point cloud correspondences</h1>\
  Hold the left mouse button and drag to rotate the scene.<br/>\
  Hold the right mouse button and drag to zoom in and out. Hold the middle mouse button and drag to pan the scene. While holding control (CTRL), click the left mouse button to select a keypoint.<br/>\
  If you need to zoom in farther, hold shift while left clicking a point to change the camera's focal point to that point. You can reset the focal point by pressing 'r'.\
  <h1>Saving keypoints</h1>\
  The same number of keypoints must be selected in both the left and right panels before the points can be saved."
  );
  help->show();
}

void CompareDescriptorsWidget::on_actionQuit_activated()
{
  exit(0);
}

// Constructor
CompareDescriptorsWidget::CompareDescriptorsWidget() : Pane(NULL), ProgressDialog(new QProgressDialog())
{
  SharedConstructor();
};

void CompareDescriptorsWidget::SharedConstructor()
{
  this->setupUi(this);

  connect(&this->FutureWatcher, SIGNAL(finished()), this->ProgressDialog , SLOT(cancel()));

  // Setup icons
  QIcon openIcon = QIcon::fromTheme("document-open");

  actionOpenPointCloud->setIcon(openIcon);
  this->toolBar_left->addAction(actionOpenPointCloud);
}

void CompareDescriptorsWidget::LoadPointCloud(const std::string& fileName)
{
  if(this->Pane)
    {
    delete this->Pane;
    }
  this->Pane = new Pane3D(this->qvtkWidget);

  vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
  reader->SetFileName(fileName.c_str());

  reader->Update();

  this->PointCloud = vtkSmartPointer<vtkPolyData>::New();
  this->PointCloud->DeepCopy(reader->GetOutput());
  
  // Start the computation.
//   QFuture<void> future = QtConcurrent::run(reader.GetPointer(), static_cast<void(vtkXMLPolyDataReader::*)()>(&vtkXMLPolyDataReader::Update));
//   this->FutureWatcher.setFuture(future);
//   this->ProgressDialog->setMinimum(0);
//   this->ProgressDialog->setMaximum(0);
//   this->ProgressDialog->setLabelText("Opening file...");
//   this->ProgressDialog->setWindowModality(Qt::WindowModal);
//   this->ProgressDialog->exec();

  this->PointCloud->GetPointData()->SetActiveScalars("Intensity");

  float range[2];
  vtkFloatArray::SafeDownCast(this->PointCloud->GetPointData()->GetArray("Intensity"))->GetValueRange(range);

  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  lookupTable->SetTableRange(range[0], range[1]);
  lookupTable->SetHueRange(0, 1);

  
  this->Pane->PointCloudMapper->SetInputConnection(this->PointCloud->GetProducerPort());
  this->Pane->PointCloudMapper->SetLookupTable(lookupTable);
  this->Pane->PointCloudActor->SetMapper(this->Pane->PointCloudMapper);
  this->Pane->PointCloudActor->GetProperty()->SetRepresentationToPoints();

  // Add Actor to renderer
  this->Pane->Renderer->AddActor(this->Pane->PointCloudActor);
  this->Pane->Renderer->ResetCamera();

  vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();

  pointPicker->PickFromListOn();
  pointPicker->AddPickList(this->Pane->PointCloudActor);
  this->Pane->qvtkWidget->GetRenderWindow()->GetInteractor()->SetPicker(pointPicker);
  this->Pane->SelectionStyle = PointSelectionStyle3D::New();
  this->Pane->SelectionStyle->Points = this->PointCloud;
  this->Pane->SelectionStyle->SetCurrentRenderer(this->Pane->Renderer);
  this->Pane->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(static_cast<PointSelectionStyle3D*>(this->Pane->SelectionStyle));

  this->Pane->Renderer->ResetCamera();

}

void CompareDescriptorsWidget::on_btnCompute_clicked()
{
  ComputeDifferences();
}

void CompareDescriptorsWidget::ComputeDifferences()
{
  vtkIdType numberOfPoints = this->Pane->PointCloudMapper->GetInput()->GetNumberOfPoints();
  std::cout << "There are " << numberOfPoints << " points." << std::endl;

  vtkIdType selectedPointId = this->Pane->SelectionStyle->SelectedPointId;
  std::cout << "selectedPointId: " << selectedPointId << std::endl;

  vtkFloatArray* descriptorArray = vtkFloatArray::SafeDownCast(this->Pane->PointCloudMapper->GetInput()->GetPointData()->GetArray(this->NameOfArrayToCompare.c_str()));

  if(!descriptorArray)
    {
    std::cerr << "Array " << this->NameOfArrayToCompare << " not found!" << std::endl;
    Helpers::OutputArrayNames(this->Pane->PointCloudMapper->GetInput());
    return;
    }
  float selectedDescriptor[descriptorArray->GetNumberOfComponents()];
  descriptorArray->GetTupleValue(selectedPointId, selectedDescriptor);

  vtkSmartPointer<vtkFloatArray> differences = vtkSmartPointer<vtkFloatArray>::New();
  differences->SetName("DescriptorDifferences");
  differences->SetNumberOfComponents(1);
  differences->SetNumberOfTuples(numberOfPoints);

  for(vtkIdType pointId = 0; pointId < this->Pane->PointCloudMapper->GetInput()->GetNumberOfPoints(); ++pointId)
    {
    float currentDescriptor[descriptorArray->GetNumberOfComponents()];
    descriptorArray->GetTupleValue(pointId, currentDescriptor);
    float difference = Helpers::ArrayDifference(selectedDescriptor, currentDescriptor, descriptorArray->GetNumberOfComponents());
    differences->SetValue(pointId, difference);
    }

  float range[2];
  differences->GetValueRange(range);
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  std::cout << "Range: " << range[0] << ", " << range[1] << std::endl;
  lookupTable->SetTableRange(range[0], range[1]);
  lookupTable->SetHueRange(0, 1);

  this->Pane->PointCloudMapper->SetLookupTable(lookupTable);
  std::cout << "UseLookupTableScalarRange " << this->Pane->PointCloudMapper->GetUseLookupTableScalarRange() << std::endl;
  this->Pane->PointCloudMapper->SetUseLookupTableScalarRange(false);
  
  this->Pane->PointCloudMapper->GetInput()->GetPointData()->SetScalars(differences);
  this->qvtkWidget->GetRenderWindow()->Render();

  vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer->SetInputConnection(this->Pane->PointCloudMapper->GetInput()->GetProducerPort());
  writer->SetFileName("Debug.vtp");
  writer->Write();
}

void CompareDescriptorsWidget::SetNameOfArrayToCompare(const std::string& nameOfArrayToCompare)
{
  this->NameOfArrayToCompare = nameOfArrayToCompare;
}

std::string CompareDescriptorsWidget::SelectArray(vtkPolyData* const polyData)
{
  vtkIdType numberOfPointArrays = polyData->GetPointData()->GetNumberOfArrays();

  for(vtkIdType i = 0; i < numberOfPointArrays; i++)
    {
    //polyData->GetPointData()->GetArrayName(i);
    }

  this->NameOfArrayToCompare = polyData->GetPointData()->GetArrayName(0);
}

void CompareDescriptorsWidget::on_actionOpenPointCloud_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Point Clouds (*.vtp)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  LoadPointCloud(fileName.toStdString());
}
