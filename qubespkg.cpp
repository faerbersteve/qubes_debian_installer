#include <iostream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include "qubespkg.h"
#include "debiancontrolfile.h"

using namespace std;

std::string qubesPkg::outputFolder{"output"};

const char* vmKernelPackage="kernel-latest-qubes-vm-6.3.9-1.qubes.fc37.x86_64.rpm";
const char* vmKernelDownload="https://ftp.qubes-os.org/repo/yum/r4.2/current/dom0/fc37/rpm/";
const char* githubUrl="https://github.com/QubesOS/{0}/archive/refs/heads/master.zip";
const char* githubPersUrl="https://github.com/faerbersteve/{0}/archive/refs/heads/master.zip";
const char* debPkgBuildCmd="dpkg-buildpackage -uc -b";
const char* debPkgEnd="_amd64.deb";
const char* debPkgEndAll="_all.deb";

inline bool file_exists (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

qubesPkg::qubesPkg(std::string projName): qubesPkg(projName,false)
{
}

qubesPkg::qubesPkg(std::string projName, bool usePersonalRepo)
{
    projectName=projName;

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
    std::string patchFile{};
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

    patchFile="../patches/"+projectName +".diff";
    //check for patchfile
    if (file_exists(patchFile))
    {
        cmd= "patch -p0 -i " + patchFile;
        ret=runCmd(cmd);

        if (ret!=0)
        {
            cout << "error while applying patch..." << endl;
            return ret;
        }
    }

    readVersion();

    cout << "unzipped" << endl;

    return 0;
}

void qubesPkg::addPackageName(std::string _pkgName, PkgInstallFlag installForBuildProc)
{ 
    pkgName* n=new pkgName();
    n->name=_pkgName;
    n->install=installForBuildProc;

    packages.push_back(n);
}

void qubesPkg::changePackageNameFlag(std::string pkgName, PkgInstallFlag installForBuildProc)
{
    for(auto p:packages)
    {
        if (p->name==pkgName)
        {
            p->install=installForBuildProc;
            break;
        }
    }
}

void qubesPkg::changePackageNameFlagAll(PkgInstallFlag installForBuildProc)
{
    for(auto p:packages)
    {
        p->install=installForBuildProc;
    }
}

int qubesPkg::createPackage()
{
    std::vector<std::string> depPkg;
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

    if (projectName=="qubes-gui-agent-linux")
    {
        cout << "Download pulsecore" << endl;
        cmd="cd " + projectName + " && ./get-latest-pulsecore.sh";
        ret=runCmd(cmd);
    }
    
    if (projectName=="qubes-linux-kernel")
    {
        //convert existing package
        if (!isPackageInstalled("alien"))
        {
            installPkgAPT("alien");
        }

        cout << "Download vm kernel package (faster way)" << endl;

        cmd="cd " + projectName + " && wget -c " + vmKernelDownload+vmKernelPackage;

        ret =runCmd(cmd);

        if (ret!=0)
        {
            cout << "error while downloading package..." << endl;
            return -1;
        }

        //convert package
        runCmd("alien -d --scripts ./" + projectName+ "/" + vmKernelPackage);

        return 0;
    }

    //check for dependencies
    debianControlFile dcf("./"+projectName +"/debian/control");

    depPkg=dcf.getBuildDependencies();

    if (depPkg.size()>0)
    {
        cout << "Check dependencies for package " << projectName << endl;

        for (auto p:depPkg)
        {
            if (!isPackageInstalled(p))
            {
                installPkgAPT(p);
            }
        }
    }

    cmd="cd " + projectName + " && " + debPkgBuildCmd;

    //    if (ignoreDependencies)
    //        cmd+=" -d";

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
        if (!all && (pkg->install==PkgInstallFlag::FOR_DEV || pkg->install==PkgInstallFlag::ALL))
        {
            //install for build process
            installPkg(pkg->name);
        }
        else if (all && (pkg->install==PkgInstallFlag::FOR_PROD || pkg->install==PkgInstallFlag::ALL))
        {
            //install for production
            installPkg(pkg->name);
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

    if (file_exists(fileName))
    {
        //can read from version file
        ifstream f(fileName, std::ifstream::in);

        std::getline(f, packageVersion);

        f.close();
    }
    else
    {
        //need to find in folder
        DIR *d;
        struct dirent *dir;
        std::string pkgName1;
        std::string tmpStr;
        int pos{0};

        pkgName1=(*packages.begin())->name;

        d=opendir(".");

        if (d)
        {
            while ((dir=readdir(d))!=NULL)
            {
                tmpStr=dir->d_name;

                //cout << "current file " << dir->d_name<<endl;

                if (tmpStr.find(pkgName1)==0)
                {
                    //found it
                    tmpStr=tmpStr.substr(pkgName1.length()+1);

                    if (tmpStr.find("dbgsym_")==0)
                    {
                        //remove dbgsym
                        tmpStr=tmpStr.substr(7);
                    }

                    if (tmpStr.find("dev_")==0)
                    {
                        //remove dev
                        tmpStr=tmpStr.substr(4);
                    }
                    
                    //cut before - or _
                    if (tmpStr.find("-")!= std::string::npos)
                        pos=tmpStr.find("-");
                    else
                        pos= tmpStr.find("_");

                    tmpStr=tmpStr.substr(0,pos);

                    packageVersion=tmpStr;
                    break;
                }
            }
            closedir(d);
        }
    }

    if (packageVersion.length()==0)
        return 1;

    cout << "Package version is " << packageVersion << endl;
    return 0;
}

void qubesPkg::initForCreate()
{
    std::string numpy="python3-numpy";
    
    runCmd("mkdir -p " + outputFolder + " && chmod 777 " + outputFolder);

    chdir(outputFolder.c_str());
    
    //fix for missing python3-numpy
    if (!isPackageInstalled(numpy))
    {
        installPkgAPT(numpy);
    }
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

    if (!file_exists(pkgFile))
    {
        //try all deb file
        pkgFile="./"+pkg+"_"+packageVersion+debPkgEndAll;
    }

    if (!file_exists(pkgFile))
    {
        for(int i=1;i <9;i++)
        {
            pkgFile="./"+pkg+"_"+packageVersion+"-"+ std::to_string(i)+debPkgEnd;

            if (file_exists(pkgFile))
                break;

            pkgFile="./"+pkg+"_"+packageVersion+"-"+ std::to_string(i)+debPkgEndAll;

            if (file_exists(pkgFile))
                break;
        }
    }
    
    //try 1 file, fix for libvchan-xen1
    if (!file_exists(pkgFile))
    {
        for(int i=1;i <9;i++)
        {
            pkgFile="./"+pkg+"1_"+packageVersion+"-"+ std::to_string(i)+debPkgEnd;

            if (file_exists(pkgFile))
                break;

            pkgFile="./"+pkg+"1_"+packageVersion+"-"+ std::to_string(i)+debPkgEndAll;

            if (file_exists(pkgFile))
                break;
        }
    }

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
    else
    {
        cout << "couldn't find file " << pkgFile << endl;
        return -1;
    }

    return 0;
}

int qubesPkg::installPkgAPT(std::string pkg)
{
    int ret{0};

    ret= runCmd("apt install -y " + pkg);

    if (ret==0)
        return 1;

    return 0;
}

int qubesPkg::removePkg(std::string pkg)
{
    std::string cmd{};

    cmd="dpkg -r " + pkg;

    return runCmd(cmd.c_str());
}

int qubesPkg::isPackageInstalled(std::string p)
{
    int r{0};

    r= runCmd("dpkg -s " + p);

    if (r!=0)
        return 0;

    return 1;
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
