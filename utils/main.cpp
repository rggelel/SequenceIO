#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageMaskSpatialObject.h"
#include "usacquisitionobject.h"

#include "vtkTransform.h"
#include "vtkMatrix4x4.h"

#include "utils/vtkXFMReader.h"
#include "utils/vtkXFMWriter.h"

#include "vtksys/CommandLineArguments.hxx"
#include <iostream>

int main(int argc, char* argv[])
{
    // Check command line arguments.
    bool printHelp = false;
    std::string movingImageFileName;
    std::string fixedImageFileName;


    vtksys::CommandLineArguments args;
    args.Initialize(argc, argv);
    args.StoreUnusedArguments(true);

    args.AddArgument("--help", vtksys::CommandLineArguments::NO_ARGUMENT, &printHelp, "Print this help.");
    args.AddArgument("--initial-transform", vtksys::CommandLineArguments::SPACE_ARGUMENT, &initialTransformFileName, "Name of the initial transform file (*.xfm).");
    args.AddArgument("--output-transform", vtksys::CommandLineArguments::SPACE_ARGUMENT, &outputTransformFileName, "Name of the output transform file (default outputTransform.xfm).");
    

    args.Parse();
    if (printHelp)
    {
        std::cerr << "Problem parsing arguments." << std::endl;
        std::cout << "Help: " << args.GetHelp() << std::endl;
        return EXIT_FAILURE;
    }

    // Reading moving image file
    MovingImageReaderType::Pointer movingReader = MovingImageReaderType::New();
    movingReader->SetFileName(movingImageFileName.c_str());
    std::cout << "Reading Moving Image... " << movingImageFileName << std::endl;
    try
    {
        movingReader->Update();
    }
    catch (itk::ExceptionObject& error)
    {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }

    ImageType::Pointer movingImage = movingReader->GetOutput();

    //Reading fixed image file 
    FixedImageReaderType::Pointer fixedReader = FixedImageReaderType::New();
    fixedReader->SetFileName(fixedImageFileName.c_str());
    std::cout << "Reading Fixed Image... " << fixedImageFileName << std::endl;
    try
    {
        fixedReader->Update();
    }
    catch (itk::ExceptionObject& error)
    {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }

    ImageType::Pointer fixedImage = fixedReader->GetOutput();

    vtkTransform* movingTransform = nullptr;

    if (!initialTransformFileName.empty())
    {
        //read initialization transform in a vtkTransform
        std::cout << "Reading Initial Transform for Moving Image... " << initialTransformFileName << std::endl;

        movingTransform = vtkTransform::New();
        vtkMatrix4x4* mat = vtkMatrix4x4::New();
        vtkXFMReader* reader = vtkXFMReader::New();
        if (reader->CanReadFile(initialTransformFileName.c_str()))
        {
            reader->SetFileName(initialTransformFileName.c_str());
            reader->SetMatrix(mat);
            reader->Update();
            reader->Delete();
        }

        movingTransform->SetMatrix(mat);

    }
*

    SequenceIO* sqIO = new SequenceIO();

    // Initialize parameters
    sqIO->SetNumberOfPixels(128000);
    sqIO->SetOrientationSelectivity(32);
    sqIO->SetPopulationSize(100);
    sqIO->SetPercentile(0.8);
    sqIO->SetUseMask(true);

    // Set image inputs
    sqIO->SetItkSourceImage(movingImage);
    sqIO->SetItkTargetImage(fixedImage);

    if (movingTransform)
    {
        sqIO->SetSourceVtkTransform(movingTransform);
    }

    // Set transform inputs
    vtkTransform* transform = vtkTransform::New();
    sqIO->SetVtkTransform(transform);

    if (movingMask)
    {
        sqIO->SetSourceMask(movingMask);
    }

    if (fixedMask)
    {
        sqIO->SetTargetMask(fixedMask);
    }



    // Write output transform
    vtkXFMWriter* transformWriter = vtkXFMWriter::New();
    transformWriter->SetFileName(outputTransformFileName.c_str());
    transformWriter->SetMatrix(transform->GetMatrix());
    transformWriter->Write();

    return EXIT_SUCCESS;
}