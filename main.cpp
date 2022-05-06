#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkTextProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>
#include <vtkNamedColors.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkLegendBoxActor.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkParametricFunctionSource.h>
#include <vtkDecimatePro.h>

#include <vtkAutoInit.h>
#include <vtkTessellatorFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellIterator.h>
#include <vtkDataSetMapper.h>
#include <vtkCubeSource.h>
#include <vtkTriangleFilter.h>
#include <vtkButterflySubdivisionFilter.h>


VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingOpenGL2);

#define vtkSPtr vtkSmartPointer
#define vtkSPtrNew(Var, Type) vtkSPtr<Type> Var = vtkSPtr<Type>::New();

using namespace std;

int main() {

	vtkSPtrNew(colors, vtkNamedColors);

	/* Read Low and high reference polygons ***************************************************************************/

	vtkSPtrNew(lowPolyReader, vtkPLYReader);
	lowPolyReader->SetFileName ("../Img/Low.ply");
	lowPolyReader->Update();
	std::cout << "Low Poly Point Count:" << lowPolyReader->GetOutput()->GetNumberOfPoints() <<endl ;
	std::cout << "Low Poly Polygons  Count:" << lowPolyReader->GetOutput()->GetNumberOfPolys() << endl;
	std::cout << "Low Poly Cell  Count:" << lowPolyReader->GetOutput()->GetNumberOfCells()  <<endl ;

	vtkSPtrNew(highPolyReader, vtkPLYReader);
	highPolyReader->SetFileName ("../Img/High.ply");
	highPolyReader->Update();
	std::cout << "High Poly Point Count:" << highPolyReader->GetOutput()->GetNumberOfPoints() <<endl ;
	std::cout << "High Poly Polygons  Count:" << highPolyReader->GetOutput()->GetNumberOfPolys() << endl;
	std::cout << "High Poly Cell  Count:" << highPolyReader->GetOutput()->GetNumberOfCells()  <<endl ;

	/* Set Mappers for reference polygons *****************************************************************************/

	vtkSPtrNew(lowPolyMapper, vtkPolyDataMapper);
	lowPolyMapper->SetInputConnection (lowPolyReader->GetOutputPort ());

	vtkSPtrNew(highPolyMapper, vtkPolyDataMapper);
	highPolyMapper->SetInputConnection (highPolyReader->GetOutputPort ());

	/* Set Mappers for reference polygons *****************************************************************************/

	vtkSPtrNew(refLowPolyActor, vtkActor);	
	refLowPolyActor->SetMapper (lowPolyMapper);
	refLowPolyActor->GetProperty()->SetColor (colors->GetColor3d("CornflowerBlue").GetData());

	vtkSPtrNew(refHighPolyActor, vtkActor);
	refHighPolyActor->SetMapper (highPolyMapper);
	refHighPolyActor->GetProperty()->SetInterpolationToFlat();
	refHighPolyActor->GetProperty()->SetColor (colors->GetColor3d("Tomato").GetData());

	/* Tessellate or Subdivide low poly mesh  **************************************************************************/

	vtkSmartPointer<vtkPolyData> originalMesh = vtkSmartPointer<vtkPolyData>::New();

	vtkNew<vtkTriangleFilter> triangles;
	triangles->SetInputConnection(lowPolyReader->GetOutputPort());
	triangles->Update();

	originalMesh = triangles->GetOutput();

	vtkNew<vtkActor> tessellateActor;

	vtkSmartPointer<vtkPolyDataAlgorithm> subdivisionFilter;
	subdivisionFilter = vtkSmartPointer<vtkButterflySubdivisionFilter>::New();
	dynamic_cast<vtkButterflySubdivisionFilter*>(subdivisionFilter.GetPointer())->SetNumberOfSubdivisions(3);

	vtkColor3d butterflyColor = colors->GetColor3d("DodgerBlue");
	tessellateActor->GetProperty()->SetDiffuseColor(butterflyColor.GetData());
	
	subdivisionFilter->SetInputData(originalMesh);
	subdivisionFilter->Update();

	vtkNew<vtkPolyDataMapper> tessellateMapper;
	tessellateMapper->SetInputConnection(subdivisionFilter->GetOutputPort());

	tessellateActor->SetMapper(tessellateMapper);

	std::cout << "Tessellate Poly Point Count:" << subdivisionFilter->GetOutput()->GetNumberOfPoints() << endl;
	std::cout << "Tessellate Poly Cell  Count:" << subdivisionFilter->GetOutput()->GetNumberOfCells() << endl;

	/* Decimate the high poly mesh  ***************************************************************************************/
	float reduction = 0.9;
	vtkSPtrNew(decimate, vtkDecimatePro);
	decimate->SetInputConnection(highPolyReader->GetOutputPort());
	decimate->SetTargetReduction(reduction);
	decimate->PreserveTopologyOn();
	decimate->Update();

	vtkSPtrNew(highDecimatePolyMapper, vtkPolyData);
	highDecimatePolyMapper->ShallowCopy(decimate->GetOutput());

	std::cout << "Decimate Poly Point Count:" << decimate->GetOutput()->GetNumberOfPoints() << endl;
	std::cout << "Decimate Poly Cell  Count:" << decimate->GetOutput()->GetNumberOfCells() << endl;

	vtkNew<vtkPolyDataMapper> decimatedMapper;
	decimatedMapper->SetInputData(highDecimatePolyMapper);

	vtkNew<vtkActor> decimatedActor;
	decimatedActor->SetMapper(decimatedMapper);
	decimatedActor->GetProperty()->SetColor(colors->GetColor3d("Tomato").GetData());
	decimatedActor->GetProperty()->SetInterpolationToFlat();

    /* Set ViewPorts for Before and After Views ************************************************************************/
	vtkSPtrNew(legend, vtkLegendBoxActor);
	legend->SetNumberOfEntries(4);
	
	legend->GetEntryTextProperty()->SetFontSize(25);

	legend->SetEntry(0, highPolyReader->GetOutput(), "High Poly Mapper", refHighPolyActor->GetProperty()->GetColor());
	legend->SetEntry(1, lowPolyReader->GetOutput(), "Low Poly Reader", refLowPolyActor->GetProperty()->GetColor());
	legend->SetEntry(2, decimate->GetOutput(), "Decimate", decimatedActor->GetProperty()->GetColor());
	legend->SetEntry(3, subdivisionFilter->GetOutput(), "Tessellate", tessellateActor->GetProperty()->GetColor());
		
	legend->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
	legend->GetPositionCoordinate()->SetValue(0.1, 0.2);
	legend->GetPosition2Coordinate()->SetCoordinateSystemToDisplay();
	legend->GetPosition2Coordinate()->SetValue(250, 120);

	legend->UseBackgroundOn();
	double background[4];
	colors->GetColor("SlateGray", background);
	legend->SetBackgroundColor(background);
	//..

	// bottom left -> Reference Low Resolution polygon
	vtkSPtrNew(renderer0, vtkRenderer);
	double BottomLeftViewport[4] = { 0.0, 0.0, 0.4, 0.5 };
	renderer0->SetViewport(BottomLeftViewport);
	renderer0->AddActor(refLowPolyActor);
	renderer0->SetBackground(colors->GetColor3d("Tomato").GetData());

	// upper left -> Reference High Resolution polygon
	vtkSPtrNew(renderer1, vtkRenderer);
	double TopLeftViewport[4] = { 0.0, 0.5, 0.4, 1.0 };
	renderer1->SetViewport(TopLeftViewport);
	renderer1->AddActor(refHighPolyActor);
	renderer1->SetBackground(colors->GetColor3d("CornflowerBlue").GetData());

	//// bottom right -> Output Tesselation
	vtkSPtrNew(renderer2, vtkRenderer);
	////...
	double BottomRightViewport[4] = { 0.4, 0.0, 0.8, 0.5 };
	renderer2->SetViewport(BottomRightViewport);
	renderer2->AddActor(tessellateActor);
	renderer2->SetBackground(colors->GetColor3d("Tomato").GetData());

	////upper right -> -> Output Decimation
	vtkSPtrNew(renderer3, vtkRenderer);	
	double TopRightViewport[4] = { 0.4, 0.5, 0.8, 1.0 };
	renderer3->SetViewport(TopRightViewport);
	renderer3->AddActor(decimatedActor);
	renderer3->SetBackground(colors->GetColor3d("CornflowerBlue").GetData());

	//// Centered
	vtkSPtrNew(renderer4, vtkRenderer);	
	double Leyenda[4] = { 0.8, 0.0, 1.0, 1.0 };
	renderer4->SetViewport(Leyenda);
	renderer4->AddActor(legend);

	vtkSPtrNew(renderWindow, vtkRenderWindow);
	renderWindow->AddRenderer (renderer0);
	renderWindow->AddRenderer (renderer1);
	renderWindow->AddRenderer (renderer2);
	renderWindow->AddRenderer (renderer3);
	renderWindow->AddRenderer (renderer4);
	renderWindow->SetSize (2000,800);
	renderWindow->SetWindowName("Decimation");

	vtkSPtrNew(renderWindowInteractor, vtkRenderWindowInteractor);
	renderWindowInteractor->SetRenderWindow (renderWindow);

	renderWindow->Render ();
	renderWindowInteractor->Start ();
	return 0;
}
