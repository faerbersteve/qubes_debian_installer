#include "debiancontrolfile.h"
#include <iostream>
#include <fstream>

debianControlFile::debianControlFile(std::string file)
{
    fileName=file;

    readFile();
}

std::vector<std::string> debianControlFile::getBuildDependencies()
{
    return buildDep;
}

void debianControlFile::readFile()
{
    std::ifstream f;
    std::string line, tmpStr;
    size_t iPos=0;
    size_t iPos2=0;
    bool buildDepBkl=false;

    f=std::ifstream(fileName, std::ifstream::in);

    while (!f.eof())
    {
        std::getline(f, line);

        if (!buildDepBkl)
        {
            iPos=line.find("Build-Depends:");

            if (iPos==0)
            {
                buildDepBkl=true;

                //package names could be named in this line
                line=line.substr(14);

                if (line.length()==0)
                    continue;
            }
        }

        if (buildDepBkl)
        {
            if (line[0]!=' ')
            {
                //end of block
                buildDepBkl=false;
                continue;
            }

            iPos=0;
            iPos2=line.find(",");

            while (iPos2 < line.length())
            {
                tmpStr=line.substr(iPos+1,iPos2-iPos-1);
                buildDep.push_back(tmpStr);

                iPos=iPos2+1;
                iPos2=line.find(",",iPos);
            }

            if (iPos+1<line.length())
            {
                tmpStr=line.substr(iPos+1);

                if (tmpStr.length()>0)
                    buildDep.push_back(tmpStr);
            }
        }
    }
    f.close();

}
