#include <iostream>
#include <fstream>
#include "json.hpp"
#include <math.h>

using json = nlohmann::json;

bool isAJsonFile(const std::string& filename) {
    if (filename.length() < 6) return false;

    std::string extension = filename.substr(filename.length() - 5);

    return extension == ".json";
}

bool isAGeojsonFile(const std::string& filename) {
    if (filename.length() < 9) return false;

    std::string extension = filename.substr(filename.length() - 8);

    return extension == ".geojson";
}

int main(int argc, char* argv[])
{
    if (argc < 3 || !isAJsonFile(argv[1]) || !isAGeojsonFile(argv[2])) {
        std::cerr << "Usage: " << argv[0] << " <input.json> <output.geojson>\n";
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    
    if (!inputFile.is_open()) {
        std::cerr << "Error openning " << argv[1] << " file!";
        return 1;
    }

    std::cout << "Converting " << argv[1] << " to " << argv[2] << ":\n";

    std::cout << "0 %";

    json inputJson = json::parse(inputFile);

    std::ofstream outFile(argv[2]);
    outFile << "{ \"type\": \"FeatureCollection\", \"features\": [\n";

    bool first = true;    

    double currentAsset = 0;
    double totalOfAssets = inputJson.size();

    for (auto& asset:inputJson) {
        double lon = asset["lon"].is_string() ? std::stod(asset["lon"].get<std::string>()) : asset["lon"].get<double>();
        double lat = asset["lat"].is_string() ? std::stod(asset["lat"].get<std::string>()) : asset["lat"].get<double>();

        json feature;
        feature["type"] = "Feature";
        feature["geometry"] = {
            {"type", "Point"},
            {"coordinates", {lon, lat}}
        };

        json properties;
        for (auto& prop:asset.items()) {
                properties[prop.key()] = prop.value();

                const std::string& key = prop.key();

                if (key != "soy_data" && key != "corn_data" && key != "rundates" && properties[key].is_array())
                {
                    for (int i = 0;i < properties[key].size();i++)
                    {
                        if (properties[key][i].is_array())
                        {
                            for (int j = 0;j < properties[key][i].size();j++)
                            {
                                std::string newKey = key + std::to_string(i) + std::to_string(j);
                                properties[newKey] = properties[key][i][j].is_null() ? json(0) : properties[key][i][j];
                            }
                        }
                        else 
                        {
                            std::string newKey = key + std::to_string(i);
                            properties[newKey] = properties[key][i].is_null() ? json(0) : properties[key][i];
                        }
                    }
                }

                if (key == "soy_data")
                {
                    for (int i = 0; i < properties["soy_data"]["forecast"]["risk"].size(); i++) {
                        properties["soy_forecast_risk" + std::to_string(i)] = properties["soy_data"]["forecast"]["risk"][i];
                    }

                    for (int i = 0; i < properties["soy_data"]["historical"]["risk"].size(); i++) {
                        properties["soy_historical_risk" + std::to_string(i)] = properties["soy_data"]["historical"]["risk"][i];
                    }

                    for (int i = 0; i < properties["soy_data"]["monitoring"]["risk"].size(); i++) {
                        properties["soy_monitoring_risk" + std::to_string(i)] = properties["soy_data"]["monitoring"]["risk"][i];
                    }
                }

                if (key == "corn_data")
                {
                    for (int i = 0; i < properties["corn_data"]["forecast"]["risk"].size(); i++) {
                        properties["corn_forecast_risk" + std::to_string(i)] = properties["corn_data"]["forecast"]["risk"][i];
                    }

                    for (int i = 0; i < properties["corn_data"]["historical"]["risk"].size(); i++) {
                        properties["corn_historical_risk" + std::to_string(i)] = properties["corn_data"]["historical"]["risk"][i];
                    }

                    for (int i = 0; i < properties["corn_data"]["monitoring"]["risk"].size(); i++) {
                        properties["corn_monitoring_risk" + std::to_string(i)] = properties["corn_data"]["monitoring"]["risk"][i];
                    }
                }
        }

        feature["properties"] = properties;

        if (!first) outFile << ",\n";
        first = false;

        outFile << feature.dump(-1);

        currentAsset++;
        double finalPercent = ceil((currentAsset / totalOfAssets) * 100);

        std::cout << "\r";
        std::cout << finalPercent << " %";
        std::cout.flush();
    }

    outFile << "\n]}";
    outFile.close();

    std::cout << "\n" << argv[1] << " converted to " << argv[2] << "!\n";
    return 0;
}