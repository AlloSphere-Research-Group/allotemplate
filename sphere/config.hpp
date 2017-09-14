#ifndef OMCONFIG_HPP
#define OMCONFIG_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace om {

/// \brief Set projector parameters and load warp and blend data
class Config
{
public:

    /// Data per projector
    struct Projector{
      int id;               ///< Projector ID
      std::string filepath; ///< path to calibration file
      int width;            ///< Projector width in pixels
      int height;           ///< Projector height in pixels
      float b,h,l,w;        ///< Viewport settings
      bool active;          ///< Active Stereo Supported boolean
      std::vector<float> data; ///< Calibration Data = numpixels * 4
      // float* data; ///< Calibration Data = numpixels * 4
    };

    /// Projectors per Render machine
    std::vector<Projector> mProjector;

    /// Print all loaded projector settings
    void print(){
        std::cout << "Projector Configuration Info" << '\n'
             << " Num Projectors: " << mProjector.size() << '\n';
        for (auto const& i : mProjector) {
            std::cout
                << "  id: " << i.id << '\n'
                << "    filepath: " << i.filepath << '\n'
                << "    width: " << i.width << '\n'
                << "    height: " << i.height << '\n'
                << "    b: " << i.b << '\n'
                << "    h: " << i.h << '\n'
                << "    l: " << i.l << '\n'
                << "    w: " << i.w << '\n'
                << "    active: " << i.active << endl;
        }
    }

    /// pass in path/to/file/
    void load(std::string filepath) {
        //Clear Projectors
        mProjector.clear();

        std::ifstream myfile {filepath};
        if (!myfile.is_open()){
            std::cout << "couldn't open file: " << filepath << std::endl;
            return;
        }
        std::cout << "Reading config file: " << filepath << std::endl;

        Projector* currently_reading_projector = nullptr;

        while (!myfile.eof()) {

            std::string line;
            getline(myfile, line);
            // config file format is [fieldname value] (separted by space)
            int sepation_index = line.find_first_of(" ");
            std::string fieldname = line.substr(0, sepation_index);
            std::stringstream fieldvalue;
            fieldvalue << line.substr(sepation_index + 1);

            if (fieldname == "id") {
                // found new projector!
                // id field should be always on the first line of the config values for single projector
                mProjector.emplace_back();
                currently_reading_projector = &mProjector.back();
                fieldvalue >> currently_reading_projector->id;
            }
            else if (!currently_reading_projector) {
                // if first line was not id, we'd be here without any currently_reading_projector
                std::cout << "config file has invalid format" << std::endl;;
            }
            else if (fieldname=="width") {
                fieldvalue >> currently_reading_projector->width;
            }
            else if (fieldname=="height") {
                fieldvalue >> currently_reading_projector->height;
            }
            else if (fieldname=="filepath") {
                fieldvalue >> currently_reading_projector->filepath;
            }
            else if (fieldname=="b") {
                fieldvalue >> currently_reading_projector->b;
            }
            else if (fieldname=="h"){
                fieldvalue >> currently_reading_projector->h;
            }
            else if (fieldname=="l"){
                fieldvalue >> currently_reading_projector->l;
            }
            else if (fieldname=="w"){
                fieldvalue >> currently_reading_projector->w;
            }
            else if (fieldname=="active"){
                fieldvalue >> currently_reading_projector->active;
            }
        }
    }

    void loadData() {
      for (auto& i : mProjector) {
        if(i.width * i.height == 0) {
            std::cout << "id " << i.id << " has no width/height data" << endl;
            continue;
        }

        ifstream dataFile {i.filepath, std::ios::binary};
        if (!dataFile.is_open()) {
            std::cout << "failed to open " << i.filepath << '\n';
            continue;
        }
        
        i.data.resize(i.width * i.height * 4);
        dataFile.read(reinterpret_cast<char*>(i.data.data()), sizeof(float) * i.data.size());

        // if(i.data) free(i.data);
        // int32_t elems = i.width * i.height;
        // i.data = (float *)malloc(sizeof(float) * elems * 4);
        // dataFile.read((char *)i.data, sizeof(float) * elems * 4);

      }
    }

    float* data(int i) {
        return mProjector[i].data.data();
        // return mProjector[i].data;
    }

    int width(int i) {
        return mProjector[i].width;
    }

    int height(int i) {
        return mProjector[i].height;
    }
};

} // namespace om

#endif