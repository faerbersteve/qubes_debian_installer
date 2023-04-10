#ifndef QUBESPKG_H
#define QUBESPKG_H
#include <string>
#include <vector>
#include <map>

enum class PkgInstallFlag
{
    ALL,
    FOR_DEV,
    FOR_PROD,
    IGNORE
};

class qubesPkg
{
public:

    qubesPkg(std::string projName);
    qubesPkg(std::string projName, bool usePersonalRepo);

    int download();

    int unzip();

    void addPackageName(std::string pkgName,PkgInstallFlag installForBuildProc);
    void changePackageNameFlag(std::string pkgName,PkgInstallFlag installForBuildProc);
    void changePackageNameFlagAll(PkgInstallFlag installForBuildProc);

    int createPackage();

    int installPackages(bool all=false);

    void cleanUp();

    int readVersion();

    static void initForCreate();
    static std::string outputFolder;

    bool usePersonalRepo{false};
    std::string projectName{};
private:
    class pkgName
    {
    public:
        std::string name;
        PkgInstallFlag install{PkgInstallFlag::IGNORE};
    };

    std::string projectUrl{};
    std::string packageVersion{};

    std::vector<pkgName*> packages{};
    std::vector<std::string> installedPkg{};

    int installPkg(std::string pkg);
    static int installPkgAPT(std::string pkg);
    int removePkg(std::string pkg);

    static int isPackageInstalled(std::string p);

    int removeFolder();

    void getProjectUrl();

    static int runCmd(std::string cmd);
};

#endif // QUBESPKG_H
