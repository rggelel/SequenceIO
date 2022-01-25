

  #include <sstream>
#include <iomanip>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <typeinfo>

#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkDataArray.h>
#include <vtkPassThrough.h>
#include <vtkImageReslice.h>
#include <itkSmartPointer.h>
#include <itkMetaDataObject.h>
#include <itkMetaDataDictionary.h>
#include <usacquisitionobject.h>




    void SequenceIOWidget::WriteAcquisition(USAcquisitionObject * usAcquisitionObject, itk::SmartPointer<ImageType> image)
    {
        m_recordfile.open(m_outputFilename.toUtf8().constData(), std::ios::out | std::ios::binary);
        if (m_recordfile)
        {
            int frameCount = usAcquisitionObject->GetNumberOfSlices();

            m_recordfile << "ObjectType = Image" << std::endl;
            m_recordfile << "NDims = 3" << std::endl;
            m_recordfile << "AnatomicalOrientation = RAS" << std::endl;
            m_recordfile << "BinaryData = True" << std::endl;
            m_recordfile << "BinaryDataByteOrderMSB = False" << std::endl;
            m_recordfile << "CenterOfRotation = 0 0 0" << std::endl;
            // TODO: add write compressed files
            m_recordfile << "CompressedData = False" << std::endl;
            m_recordfile << "DimSize = " << usAcquisitionObject->GetSliceWidth() << " "
                << usAcquisitionObject->GetSliceHeight() << " "
                << frameCount << std::endl;
            m_recordfile << "Kinds = domain domain list" << std::endl;
            m_recordfile << "ElementSpacing = 1 1 1" << std::endl;
            m_recordfile << "ElementType = MET_UCHAR" << std::endl;
            m_recordfile << "Offset = 0 0 0" << std::endl;
            m_recordfile << "TransformMatrix = 1 0 0 0 1 0 0 0 1" << std::endl;
            QString imageOrientation;
            imageOrientation = ui->ultrasoundImageOrientationComboBox->itemData(ui->ultrasoundImageOrientationComboBox->currentIndex()).toString();
            m_recordfile << "UltrasoundImageOrientation = " << imageOrientation.toUtf8().constData() << std::endl;
            // TODO: write RGB images 
            m_recordfile << "UltrasoundImageType = BRIGHTNESS" << std::endl;

            vtkSmartPointer<vtkTransform> calibrationTransform = usAcquisitionObject->GetCalibrationTransform();
            double cal[4][4];
            this->GetMatrixFromTransform(cal, calibrationTransform);
            std::string strcal = this->GetStringFromMatrix(cal);
            m_recordfile << "CalibrationTransform = " << strcal << std::endl;

            // The timestamp of the first frame is used as a baseline
            // this value is stored in `TimestampBaseline`. This is not required, but avoids sone variable overflow
            double timestampBaseline = 0.0;
            timestampBaseline = usAcquisitionObject->GetFrameTimestamp(0);
            m_recordfile << "TimestampBaseline = " << QString::number(timestampBaseline, 'f').toUtf8().constData() << std::endl;

            image = ImageType::New();

            for (int i = 0; i < frameCount; i++)
            {
                this->GetImage(usAcquisitionObject, image, i);

                double uncalmat[4][4], calmat[4][4];
                this->GetMatrixFromImage(uncalmat, image);
                this->MultiplyMatrix(calmat, uncalmat, cal);

                double timestamp = usAcquisitionObject->GetFrameTimestamp(i);
                if (timestamp > 0)
                {
                    timestamp = timestamp - timestampBaseline;
                }
                else
                {
                    timestamp = (double)i;
                }

                std::stringstream ss;
                ss << std::setw(10) << std::setfill('0') << i;
                std::string strFrameId = ss.str();
                m_recordfile << "Seq_Frame" + strFrameId + "_FrameNumber = " + strFrameId << std::endl;
                m_recordfile << "Seq_Frame" + strFrameId + "_ImageToProbeTransform = " << strcal << std::endl;
                m_recordfile << "Seq_Frame" + strFrameId + "_ImageToProbeTransformStatus = OK" << std::endl;
                m_recordfile << "Seq_Frame" + strFrameId + "_ProbeToReferenceTransform = " << this->GetStringFromMatrix(uncalmat) << std::endl;
                m_recordfile << "Seq_Frame" + strFrameId + "_ProbeToReferenceTransformStatus = OK" << std::endl;
                m_recordfile << "Seq_Frame" + strFrameId + "_ImageToReferenceTransform = " << this->GetStringFromMatrix(calmat) << std::endl;
                m_recordfile << "Seq_Frame" + strFrameId + "_ImageToReferenceTransformStatus = OK" << std::endl;
                m_recordfile << "Seq_Frame" + strFrameId + "_ImageStatus = OK" << std::endl;
                m_recordfile << "Seq_Frame" + strFrameId + "_Timestamp = " << timestamp << std::endl;
                m_recordfile << "Seq_Frame" + strFrameId + "_UnfilteredTimestamp = " << timestamp << std::endl;

                emit exportProgressUpdated(i);
            }

            m_recordfile << "ElementDataFile = LOCAL" << std::endl;

            for (int i = 0; i < frameCount; i++)
            {
                this->GetImage(usAcquisitionObject, image, i);
                typename ImageType::PixelType* pPixel = image->GetBufferPointer();
                typename ImageType::SizeType size = image->GetLargestPossibleRegion().GetSize();
                m_recordfile.write((char*)&pPixel[0], sizeof(typename ImageType::PixelType) * size[0] * size[1] * size[2]);

                emit exportProgressUpdated(i + frameCount);
            }

            m_recordfile.close();

        }
    }