#include <iostream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include "qubespkg.h"
#include "debiancontrolfile.h"

using namespace std;

std::string qubesPkg::outputFolder{"output"};
bool qubesPkg::hostIsFedora{false};
std::string qubesPkg::hostFedoraVersion{};
std::string qubesPkg::qubesDom0RepoUrl{};

const char* vmKernelPackage="kernel-latest-qubes-vm-7.0.12-1.qubes.fc41.x86_64.rpm";
const char* vmModulePackage="kernel-latest-modules-7.0.12-1.qubes.fc41.x86_64.rpm";
const char* vmKernelDownload="https://ftp.qubes-os.org/repo/yum/r4.3/current/dom0/fc41/rpm/";
const char* githubUrl="https://github.com/QubesOS/{0}/archive/refs/heads/master.zip";
const char* githubPersUrl="https://github.com/faerbersteve/{0}/archive/refs/heads/master.zip";
const char* debPkgBuildCmd="dpkg-buildpackage -uc -b";
const char* debPkgEnd="_amd64.deb";
const char* debPkgEndAll="_all.deb";

inline bool file_exists (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

inline std::string trim(const std::string& s) {
    size_t a=s.find_first_not_of(" \t\r");
    size_t b=s.find_last_not_of(" \t\r");

    if (a==std::string::npos)
        return "";

    return s.substr(a,b-a+1);
}

inline void replaceAll(std::string& line, const std::string& token, const std::string& value) {
    size_t pos=line.find(token);

    while (pos!=std::string::npos)
    {
        line=line.substr(0,pos)+value+line.substr(pos+token.length());
        pos=line.find(token,pos+value.length());
    }
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

int qubesPkg::download(bool patch)
{
    std::string cmd{};
    int ret{0};

    if (!file_exists(projectName))
    {
        cout << "Download package " << projectName << endl;

        cmd="wget -O "+ projectName +".zip -c " + projectUrl;

        runCmd(cmd);

        cout << "downloaded" << endl;

        return unzip(patch);
    }
    else
    {
        cout << "Folder exists already" << endl;

        return 0;
    }
}

int qubesPkg::unzip(bool patch)
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
    if (file_exists(patchFile) && patch)
    {
        cmd= "patch -p0 -i " + patchFile;
        ret=runCmd(cmd);

        if (ret!=0)
        {
            cout << "error while applying patch..." << endl;
            return ret;
        }
    }

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
    std::string controlFile{};
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
        cout << "Download vm kernel package (faster way)" << endl;

        cmd="cd " + projectName + " && wget -c " + vmKernelDownload+vmKernelPackage;

        ret =runCmd(cmd);

        if (ret!=0)
        {
            cout << "error while downloading package..." << endl;
            return -1;
        }

        if (hostIsFedora)
        {
            //already a native fedora rpm, just place it in the rpm output dir
            runCmd("mkdir -p rpm/" + projectName);
            runCmd("cp ./" + projectName + "/" + vmKernelPackage + " rpm/" + projectName + "/");

            return 0;
        }

        //convert existing package
        if (!isPackageInstalled("alien"))
        {
            installPkgAPT("alien");
        }

        //convert package
        runCmd("alien -d --scripts ./" + projectName+ "/" + vmKernelPackage);

        //        cout << "Download vm module package (faster way)" << endl;

        //        cmd="cd " + projectName + " && wget -c " + vmKernelDownload+vmModulePackage;

        //        ret =runCmd(cmd);

        //        if (ret!=0)
        //        {
        //            cout << "error while downloading package..." << endl;
        //            return -1;
        //        }

        //        //convert package
        //        runCmd("alien -d --scripts ./" + projectName+ "/" + vmModulePackage);

        return 0;
    }

    if (hostIsFedora)
    {
        return createRpmPackage();
    }

    controlFile = "./"+projectName +"/debian/control";
    if (!file_exists(controlFile))
    {
        cout << "error control file is missing: " << controlFile << endl;
        return -1;
    }

    //check for dependencies
    debianControlFile dcf(controlFile);

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

int qubesPkg::createRpmPackage()
{
    std::string specDir="./"+projectName+"/rpm_spec";
    std::string resultDir="rpm/"+projectName;
    std::string repoDir="rpm/repo";
    std::string mockTarget="fedora-"+hostFedoraVersion+"-x86_64";
    std::string addRepoOpt{};
    std::vector<std::string> specFiles{};
    std::string cmd{};
    std::string tmpStr{};
    DIR *d;
    struct dirent *dir;
    int ret{0};

    if (!file_exists(specDir))
    {
        cout << "no rpm_spec found for " << projectName << ", skipping" << endl;
        return 0;
    }

    d=opendir(specDir.c_str());

    if (d)
    {
        while ((dir=readdir(d))!=NULL)
        {
            tmpStr=dir->d_name;

            if (tmpStr.size()>8 && tmpStr.substr(tmpStr.size()-8)==".spec.in")
            {
                specFiles.push_back(tmpStr);
            }
        }
        closedir(d);
    }

    if (specFiles.size()==0)
    {
        cout << "no .spec.in files found for " << projectName << ", skipping" << endl;
        return 0;
    }

    runCmd("mkdir -p " + resultDir + " && chmod 777 " + resultDir);

    if (file_exists(repoDir+"/repodata"))
    {
        addRepoOpt=" --addrepo=file://$(pwd)/"+repoDir;
    }

    if (qubesDom0RepoUrl.length()>0)
    {
        addRepoOpt+=" --addrepo="+qubesDom0RepoUrl;
    }

    //optional "rel" file next to "version", same convention; defaults to "1" like the official builder
    std::string pkgRel="1";

    if (file_exists("./"+projectName+"/rel"))
    {
        ifstream relF("./"+projectName+"/rel", std::ifstream::in);
        std::getline(relF, pkgRel);
        relF.close();
        pkgRel=trim(pkgRel);
    }

    //rpm's source_date_epoch_from_changelog macro requires a non-empty %changelog
    std::string changelogEntry{};
    {
        time_t t=time(nullptr);
        struct tm* tmInfo=localtime(&t);
        char dateBuf[32];

        strftime(dateBuf,sizeof(dateBuf),"%a %b %d %Y",tmInfo);

        changelogEntry=std::string("* ")+dateBuf+" qubes_debian_installer <noreply@localhost> - "+packageVersion+"-"+pkgRel+"\n- Rebuilt by qubes_debian_installer";
    }

    for (auto specFileName : specFiles)
    {
        std::string specPathIn=specDir+"/"+specFileName;
        std::string specPathOut=specDir+"/"+specFileName.substr(0,specFileName.size()-3); //strip trailing ".in"
        std::string pkgName1{};
        std::string sourceFile{};
        std::string srpmFile{};
        std::string line{};
        size_t pos{0};

        //render the qubes-builder template placeholders (see upstream Makefile.builder/Makefile.rpmbuilder)
        ifstream inF(specPathIn, std::ifstream::in);
        ofstream outF(specPathOut, std::ofstream::out | std::ofstream::trunc);

        while (std::getline(inF, line))
        {
            replaceAll(line,"@VERSION@",packageVersion);
            replaceAll(line,"@REL@",pkgRel);
            replaceAll(line,"@BACKEND_VMM@","xen");
            replaceAll(line,"@CHANGELOG@",changelogEntry);

            outF << line << endl;
        }

        inF.close();
        outF.close();

        //parse Name:, Source0: and BuildRequires: from the rendered spec
        ifstream specF(specPathOut, std::ifstream::in);
        bool needsPython3Xen=false;

        while (std::getline(specF, line))
        {
            if (pkgName1.length()==0 && line.find("Name:")==0)
            {
                pkgName1=trim(line.substr(5));
            }
            else if (sourceFile.length()==0 && line.find("Source0:")==0)
            {
                sourceFile=trim(line.substr(8));
            }
            else if (line.find("BuildRequires:")==0 && line.find("python3-xen")!=std::string::npos)
            {
                needsPython3Xen=true;
            }
        }

        specF.close();

        //python3-xen is Qubes' own Xen python bindings package, published only in
        //the Qubes dom0 repo and only for the Fedora version Qubes currently
        //targets; no substitute exists on stock Fedora or via libvirt. Skip rather
        //than fail the whole run when that repo isn't published for this host.
        if (needsPython3Xen && qubesDom0RepoUrl.length()==0)
        {
            cout << specFileName << " needs python3-xen from the Qubes dom0 repo, not published for fc" << hostFedoraVersion << ", skipping" << endl;
            continue;
        }

        //expand the macros commonly used in Source0 (%{name}-%{version}.tar.gz)
        pos=sourceFile.find("%{name}");
        if (pos!=std::string::npos)
            sourceFile=sourceFile.substr(0,pos)+pkgName1+sourceFile.substr(pos+7);

        pos=sourceFile.find("%{version}");
        if (pos!=std::string::npos)
            sourceFile=sourceFile.substr(0,pos)+packageVersion+sourceFile.substr(pos+10);

        if (pkgName1.length()==0 || sourceFile.length()==0)
        {
            cout << "couldn't parse Name/Source0 from " << specPathOut << ", skipping" << endl;
            continue;
        }

        //build the source tarball, renaming the top-level dir to <name>-<version> to match %setup -q
        cmd="tar czf "+specDir+"/"+sourceFile+" --transform 's,^"+projectName+","+pkgName1+"-"+packageVersion+",' "+projectName;

        ret=runCmd(cmd);

        if (ret!=0)
        {
            cout << "error while creating source tarball for " << specFileName << endl;
            return -1;
        }

        //build the srpm
        cmd=mockCmdPrefix()+"mock -r "+mockTarget+" --buildsrpm --spec="+specPathOut+" --sources="+specDir+" --resultdir="+resultDir+addRepoOpt;

        ret=runCmd(cmd);

        if (ret!=0)
        {
            cout << "error while building srpm for " << specFileName << endl;
            return -1;
        }

        //find the srpm mock just produced
        d=opendir(resultDir.c_str());

        if (d)
        {
            while ((dir=readdir(d))!=NULL)
            {
                tmpStr=dir->d_name;

                if (tmpStr.size()>8 && tmpStr.substr(tmpStr.size()-8)==".src.rpm")
                {
                    srpmFile=resultDir+"/"+tmpStr;
                }
            }
            closedir(d);
        }

        if (srpmFile.length()==0)
        {
            cout << "couldn't find built srpm for " << specFileName << endl;
            return -1;
        }

        //rebuild the srpm into the binary rpm(s)
        cmd=mockCmdPrefix()+"mock -r "+mockTarget+" --rebuild "+srpmFile+" --resultdir="+resultDir+addRepoOpt;

        ret=runCmd(cmd);

        if (ret!=0)
        {
            cout << "error while building rpm for " << specFileName << endl;
            return -1;
        }
    }

    //make this project's rpms available to later projects' BuildRequires
    runCmd("mkdir -p "+repoDir);
    runCmd("cp "+resultDir+"/*.rpm "+repoDir+"/ 2>/dev/null");
    runCmd("createrepo_c --update "+repoDir);

    cout << "created" << endl;
    return 0;
}

int qubesPkg::installPackages(bool all)
{
    int ret{0};

    if (hostIsFedora)
    {
        //nothing to do: on the rpm path, build-time deps for later projects are
        //resolved by mock itself via the --addrepo local repo (see createRpmPackage()),
        //and host-level install of the built rpms isn't supported yet
        return 0;
    }

    for(auto pkg : packages)
    {
        if (!all && (pkg->install==PkgInstallFlag::FOR_DEV || pkg->install==PkgInstallFlag::ALL))
        {
            //install for build process
            ret=installPkg(pkg->name);

            if (ret!=0)
                return ret;
        }
        else if (all && (pkg->install==PkgInstallFlag::FOR_PROD || pkg->install==PkgInstallFlag::ALL))
        {
            //install for production
            ret=installPkg(pkg->name);

            if (ret!=0)
                return ret;
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

    detectHostOS();

    if (hostIsFedora)
    {
        cout << "Detected Fedora host (version " << hostFedoraVersion << "), will build rpm packages" << endl;

        runCmd("mkdir -p rpm/repo && chmod -R 777 rpm");

        if (!isRpmPackageInstalled("mock"))
        {
            installPkgDNF("mock");
        }

        if (!isRpmPackageInstalled("createrepo_c"))
        {
            installPkgDNF("createrepo_c");
        }

        //mock refuses to run outside of the mock group; the build user (delegated
        //to via runuser, see mockCmdPrefix()) needs to be a member
        const char* sudoUser=getenv("SUDO_USER");

        if (sudoUser!=nullptr)
        {
            runCmd("usermod -aG mock " + std::string(sudoUser));
        }

        //dom0-targeted specs (e.g. qubes-core-admin) BuildRequire Xen packages that
        //only exist in Qubes' own repo, not stock Fedora; only wire it in if this
        //host's Fedora version actually has a published dom0 repo for the branch
        std::string candidateRepo="https://ftp.qubes-os.org/repo/yum/r4.3/current/dom0/fc"+hostFedoraVersion+"/";

        if (runCmd("wget -q --spider "+candidateRepo+"repodata/repomd.xml")==0)
        {
            qubesDom0RepoUrl=candidateRepo;
        }
        else
        {
            cout << "No Qubes dom0 repo published for fc" << hostFedoraVersion << "; Xen-dependent packages (e.g. qubes-core-admin) may fail to resolve BuildRequires" << endl;
        }
    }
    else
    {
        //fix for missing python3-numpy
        if (!isPackageInstalled(numpy))
        {
            installPkgAPT(numpy);
        }
    }
}

void qubesPkg::detectHostOS()
{
    ifstream f("/etc/os-release", std::ifstream::in);
    std::string line{};
    std::string id{};

    if (!f.is_open())
        return;

    while (std::getline(f, line))
    {
        if (line.find("ID=")==0)
        {
            id=trim(line.substr(3));
        }
        else if (line.find("VERSION_ID=")==0)
        {
            hostFedoraVersion=trim(line.substr(11));
        }
    }

    f.close();

    if (id.size()>=2 && id.front()=='"' && id.back()=='"')
        id=id.substr(1,id.size()-2);

    if (hostFedoraVersion.size()>=2 && hostFedoraVersion.front()=='"' && hostFedoraVersion.back()=='"')
        hostFedoraVersion=hostFedoraVersion.substr(1,hostFedoraVersion.size()-2);

    hostIsFedora=(id=="fedora");
}

std::string qubesPkg::mockCmdPrefix()
{
    const char* sudoUser=getenv("SUDO_USER");

    //mock refuses to run as root; runuser (unlike sudo) can drop root to another
    //user without needing that user's password
    if (getuid()==0 && sudoUser!=nullptr)
        return "runuser -u " + std::string(sudoUser) + " -- ";

    return "";
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
            cout << "error while installing package " << pkgFile << endl;
            return -1;
        }

        installedPkg.push_back(pkg);

        return 0;
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

int qubesPkg::installPkgDNF(std::string pkg)
{
    int ret{0};

    ret= runCmd("dnf install -y " + pkg);

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

int qubesPkg::isRpmPackageInstalled(std::string p)
{
    int r{0};

    r= runCmd("rpm -q " + p + " > /dev/null 2>&1");

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
