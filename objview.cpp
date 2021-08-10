#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkDataSetAttributes.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkProgrammableFilter.h>

#include "mesh.hpp"


class timer : public vtkCommand {
public:
    timer(double tau, vtkProgrammableFilter *filter, 
          vtkRenderWindowInteractor *interactor)
        : filter{filter},
          interactor{interactor}
    {
        interactor->AddObserver(vtkCommand::TimerEvent, this);
        interactor->CreateRepeatingTimer(static_cast<unsigned long>(tau * 1000));
    }

    virtual void Execute(vtkObject *caller, unsigned long event_id,
                         void *)
    {
        if (vtkCommand::TimerEvent == event_id) {
            filter->Modified();
            interactor->Render();
        }
    }

private:
    vtkProgrammableFilter *filter;
    vtkRenderWindowInteractor *interactor;
};


struct animation_state {
    vtkProgrammableFilter *filter;
    vtkDataArray *data;
    mesh *m;
    double time;
    double tau;
};


void animate(void *data) {
    auto state = static_cast<animation_state *>(data);
    std::cout << state->time << std::endl;
    state->time += state->tau;
    for (int i = 0; i < state->m->points.size(); i++) {
        state->data->SetTuple1(i, std::sin(state->m->points[i][0] + state->time) +
                   std::cos(state->m->points[i][1] + state->time) +
                   std::sin(state->m->points[i][2] + state->time));
    }
    state->filter->GetPolyDataOutput()->CopyStructure(
        state->filter->GetPolyDataInput());
    auto point_data =
        state->filter->GetPolyDataOutput()->GetAttributes(vtkPolyData::POINT);
    point_data->SetScalars(state->data);
}


int main()
{
    vtkNew<vtkNamedColors> colors;
    std::ifstream stream{"dragon.obj"};
    auto m = parse_obj(stream);
    auto poly_data = m.to_vtk();

    vtkNew<vtkProgrammableFilter> filter;
    animation_state state{filter, vtkDataArray::CreateDataArray(VTK_DOUBLE), &m, 0.0, 0.01};
    state.data->SetNumberOfComponents(1);
    state.data->SetNumberOfTuples(m.points.size());
    filter->SetInputData(poly_data);
    filter->SetExecuteMethod(animate, &state);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(filter->GetOutputPort());
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(colors->GetColor3d("Silver").GetData());

    vtkNew<vtkRenderer> renderer;
    renderer->AddActor(actor);
    renderer->SetBackground(colors->GetColor3d("White").GetData());
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

    renderWindowInteractor->Initialize();

    timer t{state.tau, filter, renderWindowInteractor};

    renderWindow->SetSize(1024, 768);
    renderWindow->Render();

    renderWindowInteractor->Start();
}