#include <iostream>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input.geojson> <output.gejson>\n";
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    
    if (!inputFile.is_open()) {
        std::cerr << "Error openning " << argv[1] << " file!";
        return 1;
    }

    json inputJson = json::parse(inputFile);

    std::ofstream outFile(argv[2]);
    outFile << "{ \"type\": \"FeatureCollection\", \"features\": [\n";

    bool first = true;

    for (auto& asset:inputJson) {

        json feature;
        feature["type"] = "Feature";
        feature["geometry"] = {
            {"type", "Point"},
            {"coordinates", {asset["lon"], asset["lat"]}}
        };

        json properties;
        for (auto& prop:asset.items()) {
                properties[prop.key()] = prop.value();

                const std::string& key = prop.key();

                if (key == "risk9" && properties["risk9"].is_array()) {
                    for (int i = 0; i < 10; i++) {
                        std::string riskKey = "risk" + std::to_string(i);
                        if (properties.contains(riskKey) && properties[riskKey].is_array()) {
                            int j = 0;
                            for (auto& val : properties[riskKey]) {
                                properties[riskKey + std::to_string(j)] = val;
                                j++;
                            }
                        }
                    }
                }

                if (key == "rain" && prop.value().is_array()) {
                    int i = 0;
                    if (prop.value()[0].is_array()) {
                        for (auto& val : prop.value()) {
                            properties["rain" + std::to_string(i)] = val[4].is_null() ? json(0) : val[4];
                            i++;
                        }
                    }
                    else {
                        for (auto& val : prop.value()) {
                            properties["rain" + std::to_string(i)] = val.is_null() ? json(0) : val;
                            i++;
                        }
                    }
                }

                if (key == "wind" && prop.value().is_array()) {
                    int i = 0;
                    if (prop.value()[0].is_array()) {
                        for (auto& val : prop.value()) {
                            properties["wind" + std::to_string(i)] = val[4].is_null() ? json(0) : val[4];
                            i++;
                        }
                    }
                    else {
                        for (auto& val : prop.value()) {
                            properties["wind" + std::to_string(i)] = val.is_null() ? json(0) : val;
                            i++;
                        }
                    }
                }

                if (key == "tmin" && prop.value().is_array()) {
                    int i = 0;
                    for (auto& val : prop.value()) {
                        properties["tmin" + std::to_string(i)] = val[4].is_null() ? json(0) : val[4];
                        i++;
                    }
                }

                if (key == "tmax" && prop.value().is_array()) {
                    int i = 0;
                    for (auto& val : prop.value()) {
                        properties["tmax" + std::to_string(i)] = val[4].is_null() ? json(0) : val[4];
                        i++;
                    }
                }

                if (key == "flood" && prop.value().is_array()) {
                    if (prop.value().size() > 0 && prop.value().size() != 5) {
                        int i = 0;
                        for (auto& val : prop.value()) {
                            properties["flood" + std::to_string(i)] = val.is_null() ? json(0) : val;
                            i++;
                        }
                    }
                    else {
                        properties["flood0"] = prop.value().size() > 4 ? (prop.value()[4].is_null() ? json(0) : prop.value()[4]) : json(0);
                    }
                }

                std::vector<std::string> flatKeys = {
                    "fire", "pollution", "therm_confort", "landslide", "spi", "ivsc",
                    "rainpoles", "windpoles", "tminpoles", "tmaxpoles"
                };

                for (const std::string& k : flatKeys) {
                    if (key == k && prop.value().is_array()) {
                        int i = 0;
                        for (auto& val : prop.value()) {
                            properties[k + std::to_string(i)] = val.is_null() ? json(0) : val;
                            i++;
                        }
                    }
                }

                if (key == "shortWind0" && prop.value().is_array()) {
                    for (int i = 0; i < prop.value().size(); i++) {
                        properties["shortWind0" + std::to_string(i)] = prop.value()[i];  
                    }
                }

                if (key == "shortRain0" && prop.value().is_array()) {
                    for (int i = 0; i < prop.value().size(); i++) {
                        properties["shortRain0" + std::to_string(i)] = prop.value()[i];
                    }
                }

                if (key == "shortFlood0" && prop.value().is_array()) {
                    for (int i = 0; i < prop.value().size(); i++) {
                        properties["shortFlood0" + std::to_string(i)] = prop.value()[i];
                    }
                }

                std::vector<std::string> shortVars = {
                    "shortRain", "shortWind", "shortFlood", "shortPollution",
                    "shortFire", "shortTherm_confort", "shortSpi", "shortLandslide"
                };

                for (const std::string& k : shortVars) {
                    if (key == k && prop.value().is_array()) {
                        int i = 0;
                        for (auto& val : prop.value()) {
                            properties[k + std::to_string(i)] = val.is_null() ? json(0) : val;
                            i++;
                        }
                    }
                }
        }

        feature["properties"] = properties;

        if (!first) outFile << ",\n";
        first = false;

        outFile << feature.dump(-1);
    }

    outFile << "\n]}";
    outFile.close();

    std::cout << "GeoJSON written to " << argv[2] << "!";
    return 0;
}