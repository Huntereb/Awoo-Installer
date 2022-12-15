#include <minizip/unzip.h>
#include <algorithm>
#include <dirent.h>
#include <string>
#include <cstring>
#include <switch.h>
#include <sys/stat.h>
#include <unistd.h>

// https://github.com/AtlasNX/Kosmos-Updater/blob/master/source/FileManager.cpp

unz_file_info_s * _getFileInfo(unzFile unz) {
    unz_file_info_s * fileInfo = (unz_file_info_s*) malloc(sizeof(unz_file_info_s));
    unzGetCurrentFileInfo(unz, fileInfo, NULL, 0, NULL, 0, NULL, 0);
    return fileInfo;
}

std::string _getFullFileName(unzFile unz, unz_file_info_s * fileInfo) {
    char filePath[fileInfo->size_filename + 1];
    
    unzGetCurrentFileInfo(unz, fileInfo, filePath, fileInfo->size_filename, NULL, 0, NULL, 0);
    filePath[fileInfo->size_filename] = '\0';
    
    std::string path(filePath);
    path.resize(fileInfo->size_filename);

    return path;
}

bool _makeDirectoryParents(std::string path)
{
    bool bSuccess = false;
    int nRC = ::mkdir(path.c_str(), 0775);
    if(nRC == -1)
    {
        switch(errno)
        {
            case ENOENT:
                //parent didn't exist, try to create it
                if( _makeDirectoryParents(path.substr(0, path.find_last_of('/'))))
                    //Now, try to create again.
                    bSuccess = 0 == ::mkdir(path.c_str(), 0775);
                else
                    bSuccess = false;
                break;
            case EEXIST:
                //Done!
                bSuccess = true;
                break;
            default:
                bSuccess = false;
                break;
        }
    }
    else
        bSuccess = true;
    
    return bSuccess;
}

int _extractFile(const char * path, unzFile unz, unz_file_info_s * fileInfo) {
    //check to make sure filepath or fileInfo isnt null
    if (path == NULL || fileInfo == NULL)
        return -1;
        
    if (unzOpenCurrentFile(unz) != UNZ_OK)
        return -2;
    
    char folderPath[strlen(path) + 1];
    strcpy(folderPath, path);
    char * pos = strrchr(folderPath, '/');
    if (pos != NULL) {
        *pos = '\0';
        _makeDirectoryParents(std::string(folderPath));
    }
    
    u32 blocksize = 0x8000;
    u8 * buffer = (u8*) malloc(blocksize);
    if (buffer == NULL)
        return -3;
    u32 done = 0;
    int writeBytes = 0;
    FILE * fp = fopen(path, "w");
    if (fp == NULL) {
        free(buffer);
        return -4;		
    }
        
    while (done < fileInfo->uncompressed_size) {
        if (done + blocksize > fileInfo->uncompressed_size) {
            blocksize = fileInfo->uncompressed_size - done;
        }
        unzReadCurrentFile(unz, buffer, blocksize);
        writeBytes = write(fileno(fp), buffer, blocksize);
        if (writeBytes <= 0) {
            break;
        }
        done += writeBytes;
    }
    
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
    
    free(buffer);
    if (done != fileInfo->uncompressed_size)
        return -4;		
    
    unzCloseCurrentFile(unz);
    return 0;
}

namespace inst::zip {
    bool extractFile(const std::string filename, const std::string destination) {
        unzFile unz = unzOpen(filename.c_str());

        int i = 0;
        for (;;) {
            int code;
            if (i == 0) {
                code = unzGoToFirstFile(unz);
            } else {
                code = unzGoToNextFile(unz);
            }
            i++;

            if (code == UNZ_END_OF_LIST_OF_FILE) {
                break;
            } else {
                unz_file_pos pos;
                unzGetFilePos(unz, &pos);
            }

            unz_file_info_s * fileInfo = _getFileInfo(unz);

            std::string fileName = destination;
            fileName += _getFullFileName(unz, fileInfo);

            if (fileName.back() != '/') {
                int result = _extractFile(fileName.c_str(), unz, fileInfo);
                if (result < 0) {
                    free(fileInfo);
                    unzClose(unz);
                    return false;
                }
            }

            free(fileInfo);
        }

        if (i <= 0) {
            unzClose(unz);
            return false;
        }

        unzClose(unz);
        return true;
    }
}