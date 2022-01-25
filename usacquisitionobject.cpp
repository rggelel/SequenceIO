#include "vtkXFMReader.h"


bool USAcquisitionObject::Import()
{
    QStringList filenames;
    QString extension(".mnc");
    QString initialPath(Application::GetInstance().GetSettings()->WorkingDirectory);
    bool success = Application::GetInstance().GetOpenFileSequence(filenames, extension, "Select first file of acquisition", initialPath, "Minc file (*.mnc)");
    if (success)
    {
        if (this->LoadFramesFromMINCFile(filenames))
        {
            this->SetCurrentFrame(0);
            // look for calibration transform
            m_calibrationTransform->Identity();
            QFileInfo fi(filenames.at(0));
            QStringList acqSuffix = fi.fileName().split(".");
            QString acqName = tr("Acquisition_") + acqSuffix[0];
            this->SetName(acqName);
            this->SetFullFileName(fi.absoluteFilePath());
            QString calibrationTransformFileName(fi.absolutePath());
            calibrationTransformFileName.append("/calibrationTransform.xfm");
            if (QFile::exists(calibrationTransformFileName))
            {
                vtkXFMReader* reader = vtkXFMReader::New();
                if (reader->CanReadFile(calibrationTransformFileName.toUtf8()))
                {
                    vtkMatrix4x4* mat = vtkMatrix4x4::New();
                    reader->SetFileName(calibrationTransformFileName.toUtf8());
                    reader->SetMatrix(mat);
                    reader->Update();
                    m_calibrationTransform->SetMatrix(mat);
                    m_calibrationTransform->Update();
                }
                reader->Delete();
            }
        }
    }
    return success;