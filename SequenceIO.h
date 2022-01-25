// Thanks to Houssem Gueziri for writing this class

#ifndef __SequenceIO_h_
#define __SequenceIO_h_



#include <fstream>
#include "trackedsceneobject.h"
#include "ibisitkvtkconverter.h"
#include <vtkTransform.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <vtkXFMReader.h>
#include "vtk_zlib.h"

class SequenceIOPluginInterface;
class USAcquisitionObject;


namespace Ui
{
    class SequenceIOWidget;
}

class SequenceIOWidget : public QWidget
{

    OBJECT

public:

    explicit SequenceIOWidget(QWidget* parent = nullptr);
    ~SequenceIOWidget();

    void SetPluginInterface(SequenceIOPluginInterface* interf);
    void AddAcquisition(int);
    void RemoveAcquisition(int);

    struct AcqProperties
    {
        QString probeTransformName;
        QStringList trackedTransformNames;
        bool compressed;
        unsigned int compressedDataSize;
        unsigned int imageDimensions[2];
        unsigned int numberOfFrames;
        bool flippedAxes[2];
        vtkMatrix4x4* calibrationMatrix;
        bool isCalibrationFound;
        double timestampBaseline;
        QString elementDataFile;

        AcqProperties()
        {
            compressed = false;
            compressedDataSize = 0;
            imageDimensions[0] = 0;
            imageDimensions[1] = 0;
            numberOfFrames = 0;
            flippedAxes[0] = false;
            flippedAxes[1] = false;
            calibrationMatrix = vtkMatrix4x4::New();
            isCalibrationFound = false;
            timestampBaseline = 0;
        }

        bool isValid()
        {
            return imageDimensions[0] && imageDimensions[1] && numberOfFrames
                && !trackedTransformNames.isEmpty() && !elementDataFile.isEmpty();
        }
    };

private:

    template <typename ImageType> void WriteAcquisition(USAcquisitionObject*, itk::SmartPointer<ImageType>);
    bool ReadAcquisitionMetaData(QString, AcqProperties*&);
    USAcquisitionObject* ReadAcquisitionData(QString, AcqProperties*);

    std::string GetStringFromMatrix(double mat[4][4]);
    template <typename ImageType> void GetMatrixFromImage(double(&mat)[4][4], itk::SmartPointer<ImageType>);
    void GetMatrixFromTransform(double(&mat)[4][4], vtkTransform*);
    void MultiplyMatrix(double(&out)[4][4], double in1[4][4], double in2[4][4]);
    void GetImage(USAcquisitionObject*, itk::SmartPointer<IbisItkUnsignedChar3ImageType>& image, int);
    void GetImage(USAcquisitionObject*, itk::SmartPointer<IbisRGBImageType>& image, int);

    void StartProgress(int, QString title = "");
    void StopProgress();

    void UpdateAcquisitionInfoUI();

protected:


    void exportProgressUpdated(int);

};

#endif