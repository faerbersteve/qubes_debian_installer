#include <iostream>
#include <vector>
#include <string>
#include "qubespkg.h"

using namespace std;

int main()
{
    std::vector<qubesPkg*> packages{};
    int ret{0};
    bool hasError{false};

    cout << "---Qubes debian/ubuntu package creator---" << endl;

    qubesPkg::init();

    packages.push_back(new qubesPkg("qubes-core-vchan-xen","libvchan-xen"));
    packages.push_back(new qubesPkg("qubes-core-qubesdb",""));
    packages.push_back(new qubesPkg("qubes-core-qrexec",""));
    packages.push_back(new qubesPkg("qubes-rpm-oxide",""));
    packages.push_back(new qubesPkg("qubes-core-admin","",false,true)); //no debian files in qubes github repo
    packages.push_back(new qubesPkg("qubes-linux-utils","python3-qubesimgconverter"));

    packages.push_back(new qubesPkg("qubes-repo-templates",""));
    packages.push_back(new qubesPkg("qubes-gui-common",""));
    packages.push_back(new qubesPkg("qubes-gui-daemon",""));
    packages.push_back(new qubesPkg("qubes-gui-agent-linux","",false,true)); //issue with missing Trolltech.conf
    packages.push_back(new qubesPkg("qubes-core-admin-client","",false,true)); //sphinxdoc throws error
    packages.push_back(new qubesPkg("qubes-app-linux-img-converter","")); //needs python qubesimgconverter missing

    packages.push_back(new qubesPkg("qubes-artwork","")); //needs dependency qubesimgconverter
    packages.push_back(new qubesPkg("qubes-manager","",true)); //has qt5 dependency
    packages.push_back(new qubesPkg("qubes-desktop-linux-common",""));


    for(auto p :packages)
    {
        ret=p->download();

        if (ret!=0)
        {
            cout << "Error while downloading project " << p->projectName << endl;
            hasError=true;
            break;
        }

        ret=p->createPackage();

        if (ret!=0)
        {
            cout << "Error while create package of project " << p->projectName << endl;
            hasError=true;
            break;
        }

        ret=p->installDevPackage();

        if (ret!=0)
        {
            cout << "Error while install package(s) of project " << p->projectName << endl;
            hasError=true;
            break;
        }
    }

    if (hasError)
    {
        getchar();
        return ret;
    }

    cout << "Clean up" << endl;

    for(auto p :packages)
    {
        p->cleanUp();
    }

    cout << "Created all packages, ready for install" << endl;
    getchar();

    return 0;
}
