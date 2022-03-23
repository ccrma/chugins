#include "../Host.h"
#include "../pluginCtx.h"

#include <iostream>
#include <fstream>
#include <iomanip> // std::put_time
#include <ctime>

// produces a yaml dump of the requested plugin on stdout
class Dumpit : public Host
{
public:
    Dumpit() {}

    int DumpOne(char const *path, std::ostream &ostream)
    {
        std::string modpath(path);
        PluginCtx ctx;
        if(0 == this->OpenPlugin(modpath, ctx))
        {
            ctx.Print(ostream, true/*detailed*/);
            ctx.Reset();
        }
        return 0;
    }

};

static void
usage(char const *appnm)
{
    std::cout << appnm << " usage:\n"
        << "   (noargs): prints all known plugins (to stdout)\n"
        << "   <pluginfile.vst3>: dumps yaml overview of a single .vstfile (to stdout)\n"
        << "  -dstdir <dirname>: writes a new .yaml file for each plugin into dstdir\n"
        << "  -h: prints this message and exits\n"
    ;
}

int main(int argc, char* argv[])
{
    Dumpit app;
    std::string vstfile;
    std::string dstdir;
    for(int i=1;i<argc;i++)
    {
        if(!strcmp(argv[i], "-h"))
        {
            usage(argv[0]);
            return 1;
        }
        else
        if(!strcmp(argv[i], "-dstdir"))
        {
            if(++i < argc)
                dstdir = argv[i];
            else
            {
                usage(argv[0]);
                return 2;
            }
        }
        else
            vstfile = argv[i];
    }
    if(vstfile.size() == 0 && dstdir.size() == 0)
        app.PrintAllInstalledPlugins(std::cout);
    else
    if(vstfile.size() > 0)
        return app.DumpOne(vstfile.c_str(), std::cout);
    else
    {
        std::vector<std::string> plugins;
        int err = app.GetKnownPlugins(plugins);
        if(!err)
        {
            for(int i=0;i<plugins.size();i++)
            {
                std::string yamlfile = dstdir;
                std::string const &vst3file = plugins[i];
                std::size_t found = vst3file.find_last_of("/\\");
                if(found != std::string::npos)
                    yamlfile.append(vst3file.substr(found));
                else
                {
                    yamlfile.append("/");
                    yamlfile.append(vst3file);
                }
                found = yamlfile.find_last_of(".");
                if(found != std::string::npos)
                    yamlfile = yamlfile.substr(0, found);
                yamlfile.append(".yaml");
                std::filebuf fb;
                if(fb.open(yamlfile, std::ios::out))
                {
                    std::time_t t = std::time(nullptr);
                    std::tm *tm = std::localtime(&t);
                    std::ostream ostr(&fb);
                    ostr << "# This file was written by dumpVST3 on "
                         << std::put_time(tm, "%c") << "\n";
                    ostr << "# Each time you re-scan plugins it may be overwritten.\n";
                    ostr << "# Edit at your own risk.\n";
                    app.DumpOne(vst3file.c_str(), ostr);
                    fb.close();
                    std::cout << yamlfile << "\n";
                }
                else
                    std::cerr << "problem opening " << yamlfile << " for writing\n";
            }
        }
        else
            std::cerr << "No VST3 plugins found!\n";
    }
}
