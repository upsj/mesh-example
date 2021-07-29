#include <string>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#include "mesh.hpp"

int main() {
  std::ifstream stream{"dragon.obj"};
  auto m = parse_obj(stream);
  auto poly_data = m.to_vtk();
  vtkNew<vtkNamedColors> colors;
  vtkColor3d back_color = colors->GetColor3d("White");
  vtkColor3d model_color = colors->GetColor3d("Silver");
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputData(poly_data);
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetDiffuseColor(model_color.GetData());

  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(actor);
  renderer->SetBackground(back_color.GetData());
  renderer->ResetCamera();
  renderer->GetActiveCamera()->Azimuth(30);
  renderer->GetActiveCamera()->Elevation(30);
  renderer->GetActiveCamera()->Dolly(1.5);
  renderer->ResetCameraClippingRange();

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("objview");

  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderWindow->SetSize(1024, 768);
  renderWindow->Render();

  renderWindowInteractor->Start();
}