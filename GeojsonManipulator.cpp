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

    json inputJson = json::parse(inputFile);

    std::ofstream outFile(argv[2]);
    outFile << "{ \"type\": \"FeatureCollection\", \"features\": [\n";

    bool first = true;

    std::cout << "Converting " << argv[1] << " to " << argv[2] << ":\n";

    double currentAsset = 0;
    double totalOfAssets = inputJson.size();

    std::cout << "0 %";

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
                    if (prop.value()[0].is_array()) {
                        for (auto& val : prop.value()) {
                            properties["tmin" + std::to_string(i)] = val[4].is_null() ? json(0) : val[4];
                            i++;
                        }
                    }
                    else {
                        for (auto& val : prop.value()) {
                            properties["tmin" + std::to_string(i)] = val.is_null() ? json(0) : val;
                            i++;
                        }
                    }
                }

                if (key == "tmax" && prop.value().is_array()) {
                    int i = 0;
                    if (prop.value()[0].is_array()) {
                        for (auto& val : prop.value()) {
                            properties["tmax" + std::to_string(i)] = val[4].is_null() ? json(0) : val[4];
                            i++;
                        }
                    }
                    else {
                        for (auto& val : prop.value()) {
                            properties["tmax" + std::to_string(i)] = val.is_null() ? json(0) : val;
                            i++;
                        }
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