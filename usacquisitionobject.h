#ifndef __USAcquisitionObject_h_
#define __USAcquisitionObject_h_

class vtkImageData;

class USAcquisitionObject  
{
 
public:

    bool Import();
    void    SetBaseDirectory(QString dir) { m_baseDirectory = dir; }
    QString GetBaseDirectory() { return m_baseDirectory; }
    void    ExportTrackedVideoBuffer(QString destDir = "", bool masked = false, bool useCalibratedTransform = false, int relativeToID = SceneManager::InvalidId);
    bool    LoadFramesFromMINCFile(Serializer* ser);

protected:

// Importing
int m_componentsNumber;
bool m_useCalibratedTransform;
bool LoadFramesFromMINCFile(QStringList& allMINCFiles);
bool LoadGrayFrames(QStringList& allMINCFiles);
bool LoadRGBFrames(QStringList& allMINCFiles);
void AdjustFrame(vtkImageData* frame, vtkMatrix4x4* inputMatrix, vtkMatrix4x4* outputMatrix);

};