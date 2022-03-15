#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include "qubespkg.h"

using namespace std;

const char* githubUrl="https://github.com/QubesOS/{0}/archive/refs/heads/master.zip";
const char* githubPersUrl="https://github.com/faerbersteve/{0}/archive/refs/heads/master.zip";
const char* debPkgBuildCmd="dpkg-buildpackage -uc -b";
const char* debPkgEnd="-1_amd64.deb";

inline bool file_exists (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

qubesPkg::qubesPkg(std::string projName): qubesPkg(projName,false)
{
}

qubesPkg::qubesPkg(std::string projName, bool ignoreDep) : qubesPkg(projName,ignoreDep,false)
{
}

qubesPkg::qubesPkg(std::string projName, bool ignoreDep, bool usePersonalRepo)
{
    projectName=projName;

    ignoreDependencies=ignoreDep;
    this->usePersonalRepo=usePersonalRepo;

    getProjectUrl();
}

int qubesPkg::download()
{
    std::string cmd{};
    int ret{0};

    if (!file_exists(projectName))
    {
        cout << "Download package " << projectName << endl;

        cmd="wget -O "+ projectName +".zip -c " + projectUrl;

        runCmd(cmd);

        cout << "downloaded" << endl;

        return unzip();
    }
    else
    {
        cout << "Folder exists already" << endl;

        return 0;
    }
}

int qubesPkg::unzip()
{
    std::string cmd{};
    std::string folderName{};
    int ret{0};

    cout << "Unzip package " << projectName << endl;

    ret=removeFolder();

    cmd="unzip -o "+ projectName +".zip";

    ret=runCmd(cmd);

    if (ret!=0)
    {
        cout << "error while unzipping..." << endl;
        return ret;
    }

    //remove zip file
    cmd="rm "+ projectName +".zip";

    ret=runCmd(cmd);

    if (ret!=0)
    {
        cout << "error while removing zip file..." << endl;
        return ret;
    }

    folderName=projectName+"-master";

    if (!file_exists(folderName))
    {
        folderName=projectName+"-main";
    }

    cmd= "mv " + folderName+" " + projectName;

    ret=runCmd(cmd);

    if (ret!=0)
    {
        cout << "error while renaming folder..." << endl;
        return ret;
    }

    readVersion();

    cout << "unzipped" << endl;
}

void qubesPkg::addPackageName(std::string pkgName, PkgInstallFlag installForBuildProc)
{

}

int qubesPkg::createPackage()
{
    std::string cmd{};
    int ret{0};

    cout << "Create package " << projectName << endl;

    if (projectName=="qubes-python-qasync")
    {
        cmd="cd " + projectName + " && wget -c " + "https://files.pythonhosted.org/packages/source/q/qasync/qasync-0.23.0.tar.gz";

        ret=runCmd(cmd);

        cmd="cd " + projectName + " && tar -xf qasync-0.23.0.tar.gz && mv ./qasync-0.23.0/* ./";

        ret=runCmd(cmd);
    }

    cmd="cd " + projectName + " && " + debPkgBuildCmd;

    if (ignoreDependencies)
        cmd+=" -d";

    ret=runCmd(cmd);

    if (ret!=0)
    {
        cout << "error while creating package..." << endl;
        return -1;
    }

    cout << "created" << endl;
    return 0;
}

int qubesPkg::installPackages(bool all)
{
    for(auto pkg : packages)
    {
        if (!all && (pkg.second==PkgInstallFlag::FOR_DEV || pkg.second==PkgInstallFlag::ALL))
        {
            //install for build process
            installPkg(pkg.first);
        }
        else if (all && (pkg.second==PkgInstallFlag::FOR_PROD || pkg.second==PkgInstallFlag::ALL))
        {
            //install for production
            installPkg(pkg.first);
        }
    }

    return 0;
}

void qubesPkg::cleanUp()
{
    if(installedPkg.size()>0)
    {
        for(auto pkg:installedPkg)
        {
            removePkg(pkg);
        }
    }

    removeFolder();
}

int qubesPkg::readVersion()
{
    std::string fileName= "./"+ projectName+"/version";
    ifstream f(fileName, std::ifstream::in);

    std::getline(f, packageVersion);

    f.close();

    cout << "Package version is " << packageVersion << endl;
    return 0;
}

void qubesPkg::init()
{
    runCmd("mkdir -p output && chmod 777 output");

    chdir("output");
}


int qubesPkg::runCmd(std::string cmd)
{
    return system(cmd.c_str());
}

int qubesPkg::installPkg(std::string pkg)
{
    std::string pkgFile{};
    std::string cmd{};
    int ret{0};

    pkgFile="./"+pkg+"_"+packageVersion+debPkgEnd;

    if (file_exists(pkgFile))
    {
        cmd="dpkg -i " + pkgFile;

        ret=runCmd(cmd.c_str());

        if (ret!=0)
        {
            cout << "error while installing package..." << endl;
            return -1;
        }

        installedPkg.push_back(pkg);

        return 1;
    }

    return 0;
}

int qubesPkg::removePkg(std::string pkg)
{
    std::string cmd{};

    cmd="dpkg -r " + pkg;

    return runCmd(cmd.c_str());
}

int qubesPkg::removeFolder()
{
    std::string cmd{};
    int ret{0};

    cout << "Remove folder " << projectName << endl;

    cmd="rm -r ./"+ projectName;

    ret=runCmd(cmd);

    return ret;
}

void qubesPkg::getProjectUrl()
{
    std::string gUrl=githubUrl;
    int i{0};

    if (usePersonalRepo)
        gUrl=githubPersUrl;

    i=gUrl.find("{0}");

    projectUrl=gUrl.substr(0,i)+ projectName+gUrl.substr(i+3);
}
