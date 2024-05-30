#include <iostream>
#include <vector>
#include <bitset>
#include <tuple>
#include <numeric> 
#include <algorithm>
#include <random>

struct Color {
    std::bitset<24> RGB; // Гены

    Color(int r, int g, int b) {
        std::string rgb_string = std::bitset<8>(r).to_string() +
            std::bitset<8>(g).to_string() +
            std::bitset<8>(b).to_string();
        RGB = std::bitset<24>(rgb_string);
    }

    Color(std::bitset<24> tRGB) : RGB(tRGB) {}

    // Перегрузка оператора == для сравнения цветов
    bool operator==(const Color& other) const {
        return RGB == other.RGB;
    }

    // Фитофункция
    double fit(int t = 1) const {
        std::tuple<int, int, int> this_Color = this->to_rgb();
        std::tuple<int, int, int> prefer_Color(96, 96, 159);
        if (t == 1) return 1. / std::sqrt((std::get<0>(prefer_Color) - std::get<0>(this_Color)) * (std::get<0>(prefer_Color) - std::get<0>(this_Color)) +
            (std::get<1>(prefer_Color) - std::get<1>(this_Color)) * (std::get<1>(prefer_Color) - std::get<1>(this_Color)) +
            (std::get<2>(prefer_Color) - std::get<2>(this_Color)) * (std::get<2>(prefer_Color) - std::get<2>(this_Color)));
        else if (t == 2)return ((std::get<0>(prefer_Color) - std::get<0>(this_Color)) * (std::get<0>(prefer_Color) - std::get<0>(this_Color)) +
            (std::get<1>(prefer_Color) - std::get<1>(this_Color)) * (std::get<1>(prefer_Color) - std::get<1>(this_Color)) +
            (std::get<2>(prefer_Color) - std::get<2>(this_Color)) * (std::get<2>(prefer_Color) - std::get<2>(this_Color)));
        else return abs(std::get<0>(prefer_Color) - std::get<0>(this_Color)) + abs(std::get<1>(prefer_Color) - std::get<1>(this_Color)) + abs(std::get<2>(prefer_Color) - std::get<2>(this_Color));
    }

    // Оператор одноточечного кроссинговера
    Color crossover(const Color& partner, int point1) const {
        std::bitset<24> child_bits;

        // Копируем первую часть битов от первой особи
        for (int i = 0; i < point1; ++i) {
            child_bits[i] = this->RGB[i];
        }

        // Копируем третью часть битов от второй особи
        for (int i = point1; i < 24; ++i) {
            child_bits[i] = partner.RGB[i];
        }

        return Color(child_bits);
    }

    // Оператор случайной мутации
    void mutate(double Pm) {
        //генерация вероятности
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);

        for (int i = 0; i < 24; ++i) {
            if (dis(gen) < Pm) {
                RGB.flip(i);
            }
        }
    }

    // Перевод значений RGB в три значения int (R, G, B)
    std::tuple<int, int, int> to_rgb() const {
        int r = static_cast<int>((RGB.to_ulong() >> 16) & 0xFF);
        int g = static_cast<int>((RGB.to_ulong() >> 8) & 0xFF);
        int b = static_cast<int>(RGB.to_ulong() & 0xFF);
        return std::make_tuple(r, g, b);
    }
};

// кросинговер популяции
std::vector<Color> population_crossover(const std::vector<Color>& population, double Pc);

// рулеточная селекция
std::vector<Color> roulette_wheel_pop(const std::vector<Color>& population, const std::vector<double>& probabilities, int number);

// селекция отсечением
std::vector<Color> truncation_selection(const std::vector<Color>& population, const std::vector<double>& fitness_scores, int number);

int gen(int, int);

int main() {
    setlocale(LC_ALL, "ru");
    srand(time(0));

    int endYear;
    while (1) {
        std::cout << "\nВведите количество поколений: ";
        std::cin >> endYear; if (endYear >= 0 && endYear <= 10000) break;
    }
    for (int i = 10; i <= 1010; i += 100) { 
        std::cout << "\n\n" << i << " особей:\n";
        gen(i, endYear); 
    }

}

int gen(int num, int num_year) {

    // Суммирование элементов вектора double;
    auto sum = [](const std::vector<double>& vec) { return std::accumulate(vec.begin(), vec.end(), 0.0); };

    // Сравнение эдементов для sort
    auto comp = [](const Color& a, const Color& b) { return bool(a.fit() > b.fit()); };

    int num_of_colors = num; // Размер популяции
    int endYear = num_year; // Количество поколений
    int i = 0;
    
    // Генерация популяции
    std::vector<Color> population;
    for (size_t i = 0; i < (size_t)num_of_colors; i++) {
        population.push_back(Color(rand() % 255, rand() % 255, rand() % 255));
    }

    // Копия 0-й популяции для последующей эволюции
    std::vector<Color> result_population = population;

    // Значения фитнес функции особей из 0-й популяции
    std::vector<double> population_fitness_show;

    i = 0;
    for (const auto& individual : population) {
        double fitness = individual.fit();
        population_fitness_show.push_back(fitness);
    }

    // Вероятности скрещивания особей из 0-й популяции
    std::vector<double> fitness_prob_show;

    i = 0;
    for (const auto& fit : population_fitness_show) {
        double prob = fit / sum(population_fitness_show);
        fitness_prob_show.push_back(prob);
    }

    for (int year = 0; year <= endYear; year++) {

        // Фитофункция
        std::vector<double> population_fitness;
        for (const auto& individual : result_population) {
            double fitness = individual.fit();
            population_fitness.push_back(fitness);
        }

        // Шансы (для рулеточной селекции)
        std::vector<double> fitness_prob;
        for (const auto& fit : population_fitness) {
            double prob = fit / sum(population_fitness);
            fitness_prob.push_back(prob);
        }

        // Селекция
        std::vector<Color> chosen = truncation_selection(result_population, population_fitness, num_of_colors/2+1);

        // Кросинговер
        std::vector<Color> new_population = population_crossover(chosen, 0.75);

        // Сортировка от более приспособленных к менее приспособленным
        std::sort(chosen.begin(), chosen.end(), comp);
        std::sort(new_population.begin(), new_population.end(), comp);

        // Формирование нового поколения
        i = 0;
        result_population.clear();
        for (const auto& temp : chosen) {
            if (i < (chosen.size()/2+1)) {
                result_population.push_back(temp);
            }
        }

        for (const auto& temp : new_population) {
            if (i < num_of_colors) {
                result_population.push_back(temp);
            }
        }
        
        // Проверка на наличие целевой особи
        if (result_population[0].fit(-1) == 0) {
            std::cout << "Последняя популяция " << year << ":\n" << "5 лучших особей: ";
            i = 0;
            for (const auto& temp : result_population) {
                if(i < 5){
                    std::tuple<int, int, int> tup = temp.to_rgb();
                    std::cout << ++i << "#(";
                    std::cout << std::get<0>(tup) << ", " << std::get<1>(tup) << ", " << std::get<2>(tup) << ")   ";
                }
            }

            i = 0;
            for (const auto& temp : result_population) {
                if (i < 5) {
                    std::cout << "\nРазница между целевой особью и особью #" << ++i << " равна: " << temp.fit(-1);
                    std::cout << "\tФитофункция: " << temp.fit();
                }
            }
            break;
        }
        if(year == endYear) std::cout << "Особей не обнаружено\n";
    }
    
    return 0;
}

std::vector<Color> truncation_selection(const std::vector<Color>& population, const std::vector<double>& fitness_scores, int number) {
    std::vector<Color> chosen;

    // Находим индексы особей в порядке убывания их фитнес-оценок
    std::vector<size_t> sorted_indices(fitness_scores.size());
    std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
    std::sort(sorted_indices.begin(), sorted_indices.end(),
        [&fitness_scores](size_t i1, size_t i2) { return fitness_scores[i1] > fitness_scores[i2]; });

    // Выбираем особей с наивысшими фитнес-оценками
    for (int n = 0; n < number && n < static_cast<int>(population.size()); ++n) {
        chosen.push_back(population[sorted_indices[n]]);
    }

    return chosen;
}

std::vector<Color> roulette_wheel_pop(const std::vector<Color>& population, const std::vector<double>& probabilities, int number) {
    std::vector<Color> chosen;

    //генерация вероятности
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int n = 0; n < number; ++n) {
        double r = dis(gen);
        double prob_circle = 0.0;
        for (size_t i = 0; i < probabilities.size(); ++i) {
            prob_circle += probabilities[i];
            if (r <= prob_circle) {
                chosen.push_back(population[i]);
                break;
            }
        }
    }

    return chosen;
}

std::vector<Color> population_crossover(const std::vector<Color>& population, double Pc) {
    std::vector<Color> new_pop;

    //генерация вероятности
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);


    for (size_t i = 0; i < population.size(); i += 2) {
        if (Pc > dis(gen)) {

            int point = 12;
            Color child1 = population[i].crossover(population[i + 1], point);
            Color child2 = population[i + 1].crossover(population[i], point);
            child1.mutate(1. / 8); // Мутация для ребёнка 1
            child2.mutate(1. / 8); // Мутация для ребёнка 2
            new_pop.push_back(child1);
            new_pop.push_back(child2);
        }
        else {
            new_pop.push_back(population[i]);
            new_pop.push_back(population[i + 1]);
        }
    }

    return new_pop;
}