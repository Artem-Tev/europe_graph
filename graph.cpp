#include <iostream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <queue>
using json = nlohmann::json;
std::string api_key = "ULIXL2M6OXITRNCAILNVJEMCQYTQS5LF";

std::unordered_map<std::string, json> cache;

std::vector<int> bfs(const std::vector<std::vector<int>>& graph, int start) {
    int n = graph.size();
    std::vector<int> distances(n, -1);
    std::queue<int> q;

    distances[start] = 0;
    q.push(start);

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        for (int neighbor : graph[current]) {
            if (distances[neighbor] == -1) { 
                distances[neighbor] = distances[current] + 1;
                q.push(neighbor);
            }
        }
    }

    return distances;
}

std::pair<int, int> findRadiusAndDiameter(const std::vector<std::vector<int>>& graph) {
    int n = graph.size();
    std::vector<std::vector<int>> distance_matrix(n, std::vector<int>(n, -1));

    for (int i = 1; i < n; ++i) { 
        distance_matrix[i] = bfs(graph, i);
    }

    std::vector<int> eccentricities;
    for (int i = 1; i < n; ++i) {
        if (!graph[i].empty()) {
            int max_distance = *std::max_element(distance_matrix[i].begin() + 1, distance_matrix[i].end());
            if (max_distance != -1) {
                eccentricities.push_back(max_distance);
            }
        }
    }

    if (eccentricities.empty()) {
        return {-1, -1}; 
    }

    int radius = *std::min_element(eccentricities.begin(), eccentricities.end());

   
    int diameter = *std::max_element(eccentricities.begin(), eccentricities.end());

    std::cout << "Эксцентриситеты: ";
    for (int ecc : eccentricities) {
        std::cout << ecc << " ";
    }
    std::cout << "\n";

    return {radius, diameter};
}

const double EARTH_RADIUS = 6371.0; 

double haversine(double lat1, double lon1, double lat2, double lon2) { //геодезическое
    double lat1_rad = lat1 * M_PI / 180.0;
    double lon1_rad = lon1 * M_PI / 180.0;
    double lat2_rad = lat2 * M_PI / 180.0;
    double lon2_rad = lon2 * M_PI / 180.0;

    double dlat = lat2_rad - lat1_rad;
    double dlon = lon2_rad - lon1_rad;

    double a = std::pow(std::sin(dlat / 2), 2) +
               std::cos(lat1_rad) * std::cos(lat2_rad) * std::pow(std::sin(dlon / 2), 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return EARTH_RADIUS * c;
}


std::pair<double, double> getCapitalCoordinates(const std::string& countryCode) {
    std::string url = "https://restcountries.com/v3.1/alpha/" + countryCode;
    cpr::Response r = cpr::Get(cpr::Url{url});

    if (r.status_code != 200) {
        throw std::runtime_error("Ошибка при запросе данных о стране");
    }

    json response = json::parse(r.text);
    double lat = response[0]["capitalInfo"]["latlng"][0];
    double lon = response[0]["capitalInfo"]["latlng"][1];

    return {lat, lon};
}

json load_from_cache(const std::string& country_code) {
    std::string file_path = "/Users/artemtevelev/europe_graph/cache/"  + country_code + ".json";
        std::ifstream file(file_path);
        if (file.is_open()) {
            json data;
            file >> data;
            return data;
        }
    return nullptr;
}

void save_to_cache(const std::string& country_code, const json& data) {
    std::string file_path = "/Users/artemtevelev/europe_graph/cache/" + country_code + ".json";
    std::ofstream file(file_path);
    if (file.is_open()) {
        file << data.dump(4); 
    }
}
struct Country{
    int num = 0;
    std::string code;
    std::string name;
    json neighbours;
    int amount_of_neighbours;
    Country(std::string c): code(c){};
};

const std::unordered_map<std::string, std::string> countries = {
    {"AL", "Albania"},
    {"AD", "Andorra"},
    {"AT", "Austria"},
    {"BY", "Belarus"},
    {"BE", "Belgium"},
    {"BA", "Bosnia and Herzegovina"},
    {"BG", "Bulgaria"},
    {"CH", "Switzerland"},
    {"CY", "Cyprus"},
    {"CZ", "Czechia"},
    {"DE", "Germany"},
    {"DK", "Denmark"},
    {"EE", "Estonia"},
    {"ES", "Spain"},
    {"FI", "Finland"},
    {"FR", "France"},
    {"GB", "United Kingdom"},
    {"GR", "Greece"},
    {"HR", "Croatia"},
    {"HU", "Hungary"},
    {"IE", "Ireland"},
    {"IS", "Iceland"},
    {"IT", "Italy"},
    {"LT", "Lithuania"},
    {"LI", "Liechtenstein"},
    {"LU", "Luxembourg"},
    {"LV", "Latvia"},
    {"MC", "Monaco"},
    {"MD", "Moldova"},
    {"ME", "Montenegro"},
    {"MK", "North Macedonia"},
    {"MT", "Malta"},
    {"NL", "Netherlands"},
    {"NO", "Norway"},
    {"PL", "Poland"},
    {"PT", "Portugal"},
    {"RO", "Romania"},
    {"RS", "Serbia"},
    {"RU", "Russia"},
    {"SE", "Sweden"},
    {"SI", "Slovenia"},
    {"SK", "Slovakia"},
    {"SM", "San Marino"},
    {"UA", "Ukraine"},
    {"VA", "Vatican City"},
    {"TR", "Turkey"}
};
int main() {
    std::vector<Country> sp;
    for (const auto& [code, name] : countries) {
        cpr::Parameters params = {
            {"key", api_key},
            {"country_code", code},
            {"format", "json"}
        };

        Country x(code);
        x.name = name;

        if (cache.find(code) != cache.end()) {
            x.neighbours["Соседи"] = cache[code];
        }
        else if (json cached_data = load_from_cache(code); cached_data != nullptr) {
            x.neighbours["Соседи"] = cached_data;
            cache[code] = cached_data;
        }
        else {
            cpr::Response r = cpr::Get(cpr::Url{"https://api.geodatasource.com/v2/neighboring-countries"}, params);
            json response = json::parse(r.text);
            x.neighbours["Соседи"] = response;
            cache[code] = response;
            save_to_cache(code, response); 
        }

        sp.push_back(x);
    }
    std::vector<std::vector<int>> graph(46);
    int numbr = 1;
    for(auto& i: sp){
        std::cout<<numbr<<") "<<i.name<<':';
            int cnt=0;
            for(const auto& j: i.neighbours["Соседи"]){
                if(j["country_name"].get<std::string>().length() ){
                    if(countries.find(j["country_code"].get<std::string>()) != countries.end()){
                        std::cout<<" || "<<j["country_name"].get<std::string>();
                        cnt++;
                    }
                }
                else 
                    std::cout<<" У страны нет соседей";
            }
            if(cnt) {
                i.amount_of_neighbours = cnt;
                i.num =numbr;
                std::cout<<" || Кол-во соседей: "<<i.amount_of_neighbours;
                }
            std::cout<<"\n\n";
            numbr++;
    }
    std::vector<std::vector<int>> sp_num(sp.size() + 1);
    for (const auto& country : sp) {
        int from = country.num;
        for (const auto& neighbour : country.neighbours["Соседи"]) {
            std::string neighbour_code = neighbour["country_code"].get<std::string>();
            if (countries.find(neighbour_code) != countries.end()) {
                for (const auto& c : sp) {
                    if (c.code == neighbour_code) {
                        int to = c.num; 
                        sp_num[from].push_back(to); 
                        break;
                    }
                }
            }
        }
    }
    
    std::cout << "Список смежности графа:\n";
    for (size_t i = 1; i < sp_num.size(); ++i) {
        std::cout << i << ": ";
        for (int neighbour : sp_num[i]) {
            std::cout << neighbour << " ";
        }
        std::cout << "\n";
    }

    int max_neighbours = 0;
    for(auto& i: sp){
        if (i.amount_of_neighbours > max_neighbours) max_neighbours = i.amount_of_neighbours;
    }
    int edges = 0;
    for(auto& i: sp){
        edges += i.amount_of_neighbours;
    }
    std::map <float, std::string> edges_sp;
    for (int i = 0; i < sp.size(); i++) {
        for (int j = 0; j < sp_num[i + 1].size(); j++) {
            int neighbor_index = sp_num[i + 1][j] - 1; 
            auto [lat, lng] = getCapitalCoordinates(sp[i].code);
            auto [lat1, lng1] = getCapitalCoordinates(sp[neighbor_index].code);
            float dist = haversine(lat, lng, lat1, lng1);
            edges_sp[dist] = sp[i].name + " and " + sp[neighbor_index].name;
        }
    }

    std::cout<<"\n|V|="<<numbr-1;
    std::cout<<"\n|E|="<<edges/2;
    std::cout<< "\nМаксимальное кол-во соседей: "<<max_neighbours;
    std::pair<int, int> r_and_d = findRadiusAndDiameter(sp_num);
    std::cout<<"\nr="<<r_and_d.first;
    std::cout<<"\nd="<<r_and_d.second;
    std::cout<<"\nРасстояния:\n"
    for(auto i: edges_sp){
        std::cout<<i.second<<":"<<i.first;
        std::cout<<'\n';
    }
    return 0;
}
