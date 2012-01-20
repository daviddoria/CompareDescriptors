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

void CompareDescriptorsWidget::on_actionHelp_activated()
{
  QTextEdit* help=new QTextEdit();

  help->setReadOnly(true);
  help->append("<h1>Compare descriptors</h1>\
  Load a point cloud <br/>\
  Ctrl+click to select a point. <br/>\
  Click Compare.<br/>"
  );
  help->show();
}

void CompareDescriptorsWidget::on_actionQuit_activated()
{
  exit(0);
}

// Constructor
CompareDescriptorsWidget::CompareDescriptorsWidget() : MarkerRadius(.05)
{
  this->ProgressDialog = new QProgressDialog();
  SharedConstructor();
};

void CompareDescriptorsWidget::SharedConstructor()
{
  this->setupUi(this);

  this->PointPicker = vtkSmartPointer<vtkPointPicker>::New();
  this->PointPicker->PickFromListOn();

  // Point cloud
  this->PointCloud = vtkSmartPointer<vtkPolyData>::New();

  this->PointCloudMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->PointCloudMapper->SetInputConnection(this->PointCloud->GetProducerPort());

  this->PointCloudActor = vtkSmartPointer<vtkActor>::New();
  this->PointCloudActor->SetMapper(this->PointCloudMapper);

  // Marker
  this->MarkerSource = vtkSmartPointer<vtkSphereSource>::New();
  this->MarkerSource->SetRadius(this->MarkerRadius);
  this->MarkerSource->Update();

  this->MarkerMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->MarkerMapper->SetInputConnection(this->MarkerSource->GetOutputPort());

  this->MarkerActor = vtkSmartPointer<vtkActor>::New();
  this->MarkerActor->SetMapper(this->MarkerMapper);

  // Renderer
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->Renderer->AddActor(this->PointCloudActor);
  this->Renderer->AddActor(this->MarkerActor);

  this->SelectionStyle = PointSelectionStyle3D::New();
  this->SelectionStyle->AddObserver(this->SelectionStyle->SelectedPointEvent, this, &CompareDescriptorsWidget::SelectedPointCallback);

  // Qt things
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  connect(&this->FutureWatcher, SIGNAL(finished()), this->ProgressDialog , SLOT(cancel()));

  // Setup icons
  QIcon openIcon = QIcon::fromTheme("document-open");

  actionOpenPointCloud->setIcon(openIcon);
  this->toolBar_left->addAction(actionOpenPointCloud);
}

void CompareDescriptorsWidget::SelectedPointCallback(vtkObject* caller, long unsigned int eventId, void* callData)
{
  double p[3];
  this->PointCloud->GetPoint(this->SelectionStyle->SelectedPointId, p);
  this->MarkerActor->SetPosition(p);
}

void CompareDescriptorsWidget::Refresh()
{
  this->qvtkWidget->GetRenderWindow()->Render();
}

// void CompareDescriptorsWidget::on_cmbArrayName_activated(int value)
// {
//   this->NameOfArrayToCompare = this->comboBox->currentText().toStdString();
// }

void CompareDescriptorsWidget::LoadPointCloud(const std::string& fileName)
{
  vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
  reader->SetFileName(fileName.c_str());

  reader->Update();

  this->PointCloud->DeepCopy(reader->GetOutput());

  // Start the computation.
//   QFuture<void> future = QtConcurrent::run(reader.GetPointer(), static_cast<void(vtkXMLPolyDataReader::*)()>(&vtkXMLPolyDataReader::Update));
//   this->FutureWatcher.setFuture(future);
//   this->ProgressDialog->setMinimum(0);
//   this->ProgressDialog->setMaximum(0);
//   this->ProgressDialog->setLabelText("Opening file...");
//   this->ProgressDialog->setWindowModality(Qt::WindowModal);
//   this->ProgressDialog->exec();

  this->PointCloudMapper->SetInputConnection(this->PointCloud->GetProducerPort());

  this->PointCloudActor->GetProperty()->SetRepresentationToPoints();

  this->Renderer->ResetCamera();

  this->PointPicker->AddPickList(this->PointCloudActor);

  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetPicker(this->PointPicker);
  this->SelectionStyle->Points = this->PointCloud;
  this->SelectionStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->SelectionStyle);

  this->Renderer->ResetCamera();

  PopulateArrayNames(this->PointCloud);
}

void CompareDescriptorsWidget::PopulateArrayNames(vtkPolyData* const polyData)
{
  this->cmbArrayName->clear();
  vtkIdType numberOfPointArrays = polyData->GetPointData()->GetNumberOfArrays();

  for(vtkIdType i = 0; i < numberOfPointArrays; i++)
    {
    std::string arrayName = polyData->GetPointData()->GetArrayName(i);
    this->cmbArrayName->addItem(arrayName.c_str());
    }
}

void CompareDescriptorsWidget::on_btnCompute_clicked()
{
  ComputeDifferences();
}

void CompareDescriptorsWidget::ComputeDifferences()
{
  vtkIdType numberOfPoints = this->PointCloud->GetNumberOfPoints();
  std::cout << "There are " << numberOfPoints << " points." << std::endl;

  vtkIdType selectedPointId = this->SelectionStyle->SelectedPointId;
  std::cout << "selectedPointId: " << selectedPointId << std::endl;

  if(selectedPointId < 0 || selectedPointId >= numberOfPoints)
    {
    std::cerr << "You must select a point to compare!" << std::endl;
    return;
    }

  std::cout << "Point cloud address: " << this->PointCloud << std::endl;
  std::cout << "Number of arrays: " << this->PointCloud->GetPointData()->GetNumberOfArrays() << std::endl;
  
  std::string nameOfArrayToCompare = this->cmbArrayName->currentText().toStdString();
  std::cout << "nameOfArrayToCompare: " << nameOfArrayToCompare << std::endl;
  vtkDataArray* descriptorArray = this->PointCloud->GetPointData()->GetArray(nameOfArrayToCompare.c_str());

  if(!descriptorArray)
    {
    std::string errorString = "Array " + nameOfArrayToCompare + " not found!";
    throw std::runtime_error(errorString); // The array should always be found because we are selecting it from a list of available arrays!
    }

  double* selectedDescriptor = new double[descriptorArray->GetNumberOfComponents()];
  descriptorArray->GetTuple(selectedPointId, selectedDescriptor);
  std::cout << "selectedDescriptor: " << selectedDescriptor[0] << " " << selectedDescriptor[1] << std::endl;

  vtkSmartPointer<vtkFloatArray> differences = vtkSmartPointer<vtkFloatArray>::New();
  differences->SetName("DescriptorDifferences");
  differences->SetNumberOfComponents(1);
  differences->SetNumberOfTuples(numberOfPoints);

  for(vtkIdType pointId = 0; pointId < this->PointCloud->GetNumberOfPoints(); ++pointId)
    {
    double* currentDescriptor = descriptorArray->GetTuple(pointId);
    //std::cout << "descriptor " << pointId << " : " << currentDescriptor[0] << " " << currentDescriptor[1] << std::endl;
    float difference = Helpers::ArrayDifference(selectedDescriptor, currentDescriptor, descriptorArray->GetNumberOfComponents());
    differences->SetValue(pointId, difference);
    }

  this->PointCloud->GetPointData()->AddArray(differences);
  this->PointCloud->GetPointData()->SetActiveScalars("DescriptorDifferences");

  float range[2];
  differences->GetValueRange(range);
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  std::cout << "Range: " << range[0] << ", " << range[1] << std::endl;
  lookupTable->SetTableRange(range[0], range[1]);
  lookupTable->SetHueRange(0, 1);

  this->PointCloudMapper->SetLookupTable(lookupTable);
  //std::cout << "UseLookupTableScalarRange " << this->Pane->PointCloudMapper->GetUseLookupTableScalarRange() << std::endl;
  //this->Pane->PointCloudMapper->SetUseLookupTableScalarRange(false);

  // Without this, only a small band of colors is produce around the point.
  // I'm not sure why the scalar range of the data set is not the same?
  this->PointCloudMapper->SetUseLookupTableScalarRange(true);

  this->qvtkWidget->GetRenderWindow()->Render();
  //PopulateArrayNames(this->PointCloud);
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
