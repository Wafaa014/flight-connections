#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include <sstream>
#include <map>
#include <ctime>
#include <random>

using namespace std;

clock_t start;
int totalTime;
int number_of_areas;
vector<string> area;
vector<string> airport;
vector<vector<string>> airport_in_area;
vector<vector<int>> airport_in_area_index;
vector<vector<vector<int>>> cost;
string first_airport;
map<string, int> m_area;
map<string, int> m_airport;
map<string, int> m_airport_in_area;
map<int, int> m_non_feas;
map<int, int> m_non_feasp;
int number_of_operators;

template <class Container>
void split1(const string& str, Container& cont) {
	istringstream iss(str);
	copy(istream_iterator<string>(iss),
		istream_iterator<string>(),
		back_inserter(cont));
}

int getAreaIndexFromAirport(string air) {
	return m_area[air];
}

int getAirportIndex_from_airport(string air) {
	return m_airport[air];
}

int getAirportIndex_from_airport_in_area(string air) {
	return m_airport_in_area[air];
}

void mapping() {
	for (int i = 0; i < airport_in_area.size(); ++i) {
		for (int j = 0; j < airport_in_area[i].size(); ++j) {
			m_area[airport_in_area[i][j]] = i;
			m_airport_in_area[airport_in_area[i][j]] = j;
		}
	}

	for (int i = 0; i < airport.size(); ++i) {
		m_airport[airport[i]] = i;
	}
}

void ReadFile(const char* input_file_name) {
	string a;
	string b;
	vector<string> temp_string;
	ifstream fileReader(input_file_name);
	if (fileReader.is_open()) {
		fileReader >> number_of_areas >> first_airport;
		for (int i = 0; i < number_of_areas; i++) {
			getline(fileReader, a);
			while (a.size() == 0) {
				getline(fileReader, a);
			}
			area.push_back(a);
			getline(fileReader, b);
			while (b.size() == 0) {
				getline(fileReader, b);
			}
			split1(b, temp_string);
			airport_in_area.push_back(temp_string);
			temp_string.clear();
		}

		vector<int> airports;
		for (int i = 0; i < airport_in_area.size(); ++i) {
			for (int j = 0; j < airport_in_area[i].size(); ++j) {
				airports.push_back(j);
			}
			airport_in_area_index.push_back(airports);
			airports.clear();
		}

		for (int i = 0; i < airport_in_area.size(); i++) {
			for (int j = 0; j < airport_in_area[i].size(); j++) {
				airport.push_back(airport_in_area[i][j]);
			}
		}

		mapping();

		cost.resize(airport.size());
		for (int i = 0; i < cost.size(); ++i) {
			cost[i].resize(airport.size());
			for (int j = 0; j < cost[i].size(); ++j) {
				cost[i][j].resize(number_of_areas);
				for (int k = 0; k < cost[i][j].size(); ++k) {
					cost[i][j][k] = -1;
				}
			}
		}

		string departure, arrival;
		int day, costt;
		while (!fileReader.eof()) {
			fileReader >> departure;
			fileReader >> arrival;
			fileReader >> day;
			fileReader >> costt;
			int departure_index = getAirportIndex_from_airport(departure);
			int arrival_index = getAirportIndex_from_airport(arrival);
			if (day == 0) {
				for (int k = 0; k < number_of_areas; k++) {
					if ((cost[departure_index][arrival_index][k] == -1)
						|| (costt < cost[departure_index][arrival_index][k])) {
						cost[departure_index][arrival_index][k] = costt;
					}
				}
			}
			else if ((cost[departure_index][arrival_index][day - 1] == -1)
				|| (costt < cost[departure_index][arrival_index][day - 1])) {
				cost[departure_index][arrival_index][day - 1] = costt;
			}
		}
	}
	else {
		cout << "error opening file " << endl;
		system("pause");
		exit(0);
	}
}

int CalculateFeasibility(vector<vector<int>> sol) {
	int feas = 0;
	for (int k = 0; k < number_of_areas; k++) {
		if (cost[getAirportIndex_from_airport(airport_in_area[sol[0][k]][sol[1][k]])]
			[getAirportIndex_from_airport(airport_in_area[sol[0][k + 1]][sol[1][k + 1]])][k]
			== -1) {
			feas++;
			m_non_feas.insert({ k,-1 });
		}
	}
	return feas;
}

void calculate_feas_partial(int day, vector<vector<int>> sol) {
	int cost_k = cost[getAirportIndex_from_airport(airport_in_area
		[sol[0][day - 1]][sol[1][day - 1]])]
		[getAirportIndex_from_airport(airport_in_area[sol[0][day]][sol[1][day]])][day - 1];

	int cost_k_plus_1 = cost[getAirportIndex_from_airport(airport_in_area
		[sol[0][day]][sol[1][day]])]
		[getAirportIndex_from_airport(airport_in_area[sol[0][day + 1]][sol[1][day + 1]])][day];

	if (cost_k == -1) {
		map<int, int>::iterator it = m_non_feas.find(day - 1);
		if (it == m_non_feas.end())
			m_non_feas.insert({ day - 1,-1 });
	}
	else if (cost_k != -1) {
		m_non_feas.erase(day - 1);
	}

	if (cost_k_plus_1 == -1) {
		map<int, int>::iterator it = m_non_feas.find(day);
		if (it == m_non_feas.end())
			m_non_feas.insert({ day,-1 });
	}
	else if (cost_k_plus_1 != -1) {
		m_non_feas.erase(day);
	}
}

void calculate_feas3_partial(int day, vector<vector<int>> sol) {
	int cost_k = cost[getAirportIndex_from_airport(airport_in_area
		[sol[0][day - 1]][sol[1][day - 1]])]
		[getAirportIndex_from_airport(airport_in_area[sol[0][day]][sol[1][day]])][day - 1];

	if (cost_k == -1) {
		map<int, int>::iterator it = m_non_feas.find(day - 1);
		if (it == m_non_feas.end())
			m_non_feas.insert({ day - 1, -1 });
	}
	else if (cost_k != -1) {
		m_non_feas.erase(day - 1);
	}
}

vector<vector<int>> CopyFromTo_sol(vector<vector<int>> sol1, vector<vector<int>> sol2) {
	sol2 = sol1;
	return sol2;
}

map<int, int> CopyFromTo_map(map<int, int> map1, map<int, int> map2) {
	map2 = map1;
	return map2;
}

vector<vector<int>> operator_swap(vector<vector<int>> sol) {
	int loc1 = rand() % (number_of_areas - 1) + 1;
	int loc2 = rand() % (number_of_areas - 1) + 1;

	swap(sol[0][loc1], sol[0][loc2]);
	swap(sol[1][loc1], sol[1][loc2]);

	if (rand() % 2 == 0) {
		sol[1][loc1] = rand() % airport_in_area_index[sol[0][loc1]].size();
	}
	if (rand() % 2 == 0) {
		sol[1][loc2] = rand() % airport_in_area_index[sol[0][loc2]].size();
	}

	calculate_feas_partial(loc1, sol);
	calculate_feas_partial(loc2, sol);
	return sol;
}

vector<vector<int>> operator_change_airport(vector<vector<int>> sol) {
	int loc = rand() % (number_of_areas)+1;
	int airport = rand() % airport_in_area_index[sol[0][loc]].size();
	sol[1][loc] = airport;
	if (loc == number_of_areas) {
		calculate_feas3_partial(loc, sol);
	}
	else {
		calculate_feas_partial(loc, sol);
	}

	return sol;
}

vector<vector<int>> operator_insert(vector<vector<int>> sol) {
	int loc = rand() % (number_of_areas - 1) + 1;
	int index = rand() % (number_of_areas - 1) + 1;

	vector<int>::iterator it_area = sol[0].begin();
	vector<int>::iterator it_airport = sol[1].begin();

	if (loc != index) {
		sol[0].insert(it_area + index, sol[0][loc]);
		sol[1].insert(it_airport + index, sol[1][loc]);
		if (loc < index) { // 0 1 2 4 5 6 7 3 8 9 0
			for (int i = loc; i <= number_of_areas; i++) {
				sol[0][i] = sol[0][i + 1];
				sol[1][i] = sol[1][i + 1];
			}
			for (int i = loc; i <= index; i++) {
				calculate_feas3_partial(i, sol);
			}
		}
		else if (loc > index) {  // 0 1 2 8 3 4 5 6 7 9 0 
			for (int i = loc + 1; i <= number_of_areas; i++) {
				sol[0][i] = sol[0][i + 1];
				sol[1][i] = sol[1][i + 1];
			}
			for (int i = index; i <= loc + 1; i++) {
				calculate_feas3_partial(i, sol);
			}
		}
	}
	return sol;
}

vector<vector<int>> operator_reverse(vector<vector<int>> sol) {
	int loc1 = rand() % (number_of_areas - 1) + 1;
	int loc2 = rand() % (number_of_areas - 1) + 1;

	// 0 1 2 3 4 5 6 7 8 9
	// 0 1 2 3 4 5 6 8 7 9
	// 0 1 7 6 5 4 3 2 8 9

	int i = abs(loc2 - loc1);
	if (i % 2 == 0) { i = i / 2; }
	else { i = (i + 1) / 2; }
	if (loc2 > loc1) {
		for (int j = 0; j < i; j++) {
			swap(sol[0][loc1 + j], sol[0][loc2 - j]);
			swap(sol[1][loc1 + j], sol[1][loc2 - j]);
		}
		for (int k = loc1; k <= loc2 + 1; k++) {
			calculate_feas3_partial(k, sol);
		}
	}
	else if (loc2 < loc1) {
		for (int j = 0; j < i; j++) {
			swap(sol[0][loc2 + j], sol[0][loc1 - j]);
			swap(sol[1][loc2 + j], sol[1][loc1 - j]);
		}
		for (int k = loc2; k <= loc1 + 1; k++) {
			calculate_feas3_partial(k, sol);
		}
	}
	return sol;
}

vector<vector<int>> ApplyOperator(vector<vector<int>> sol, int h) {
	if (h == 0)
		return operator_swap(sol);
	else if (h == 1)
		return operator_change_airport(sol);
	else if (h == 2)
		return operator_insert(sol);
	else if (h == 3)
		return operator_reverse(sol);
}

int TotalCost(vector<vector<int>> sol) {
	int total = 0;
	for (int k = 0; k < number_of_areas; k++) {
		total += cost[getAirportIndex_from_airport(airport_in_area[sol[0][k]][sol[1][k]])]
			[getAirportIndex_from_airport(airport_in_area[sol[0][k + 1]][sol[1][k + 1]])][k];
	}
	return total;
}

vector<vector<int>> InitialiseSol() {

	vector<vector<int>> solution(2, vector<int>(number_of_areas + 1));
	vector<vector<int>> previous_solution(2, vector<int>(number_of_areas + 1));
	vector<int> areas;

	//first area
	int first_airport_index = getAirportIndex_from_airport_in_area(first_airport);
	int first_airport_area_index = getAreaIndexFromAirport(first_airport);
	solution[0][0] = first_airport_area_index;
	solution[1][0] = first_airport_index;
	areas.push_back(solution[0][0]);

	//last area
	solution[0][number_of_areas] = solution[0][0];
	solution[1][number_of_areas] = rand() % airport_in_area_index[solution[0][0]].size();

	for (int j = 1; j < (solution[0].size() - 1); j++) {
		solution[0][j] = -1;
	}

	int least_cost = 0;

	for (int j = 1; j < (solution[0].size() - 1); j++) {

		for (int i = 0; i < airport_in_area_index.size(); i++) {

			if (!(find(areas.begin(), areas.end(), i) != areas.end())) {//if not found

				//int air = rand() % airport_in_area_index[i].size();

				for (int air = 0; air < airport_in_area_index[i].size(); air++) {

					if (cost[getAirportIndex_from_airport(airport_in_area[solution[0][j - 1]]
						[solution[1][j - 1]])]
						[getAirportIndex_from_airport(airport_in_area[i][air])][j - 1] != -1) {

						if (solution[0][j] == -1) {
							least_cost = cost[getAirportIndex_from_airport(airport_in_area
								[solution[0][j - 1]]
							[solution[1][j - 1]])]
								[getAirportIndex_from_airport(airport_in_area[i][air])][j - 1];

							solution[0][j] = i;
							solution[1][j] = air;
						}

						if (cost[getAirportIndex_from_airport(airport_in_area
							[solution[0][j - 1]][solution[1][j - 1]])]
							[getAirportIndex_from_airport(airport_in_area[i][air])][j - 1]
							< least_cost) {

							least_cost = cost[getAirportIndex_from_airport(airport_in_area
								[solution[0][j - 1]]
							[solution[1][j - 1]])]
								[getAirportIndex_from_airport(airport_in_area[i][air])][j - 1];

							solution[0][j] = i;
							solution[1][j] = air;
						}

					}

				}
			}
		}

		if (solution[0][j] == -1) {
			int ar = rand() % airport_in_area_index.size();
			while ((find(areas.begin(), areas.end(), ar) != areas.end())) {// if found
				ar = rand() % airport_in_area_index.size();
			}

			int air = rand() % airport_in_area_index[ar].size();
			solution[0][j] = ar;
			solution[1][j] = air;
			areas.push_back(ar);
		}
		else if (solution[0][j] != -1) {
			areas.push_back(solution[0][j]);
		}

	}

	int feas = CalculateFeasibility(solution);
	previous_solution = CopyFromTo_sol(solution, previous_solution);
	m_non_feasp = CopyFromTo_map(m_non_feas, m_non_feasp);
	int nfeas;

	while (feas != 0 && (float)(clock() - start) / CLOCKS_PER_SEC <= totalTime) {

		int h = rand() % number_of_operators;
		solution = ApplyOperator(solution, h);
		nfeas = m_non_feas.size();
		if (nfeas <= feas) {
			feas = nfeas;
			previous_solution = CopyFromTo_sol(solution, previous_solution);
			m_non_feasp = CopyFromTo_map(m_non_feas, m_non_feasp);
		}
		else {
			solution = CopyFromTo_sol(previous_solution, solution);
			m_non_feas = CopyFromTo_map(m_non_feasp, m_non_feas);
		}
	}
	return solution;
}

vector<vector<int>> ImproveInitialSolution(vector<vector<int>> sol) {
	// Iterated local search
	// Hyper-heuristic
	// Simulated annealing
	// Genetic algorithm

	m_non_feasp = CopyFromTo_map(m_non_feas, m_non_feasp);

	vector<vector<int>> sol_prev = CopyFromTo_sol(sol, sol_prev);
	vector<vector<int>> best_sol = CopyFromTo_sol(sol, best_sol);

	int cost, cost_prev, best_cost;
	cost_prev = TotalCost(sol_prev);
	best_cost = TotalCost(best_sol);

	float temp = 1000;
	//float temp_modifier = 0.99;
	float rate_of_cooling = 0.005;

	while (((float)(clock() - start)) / CLOCKS_PER_SEC <= totalTime) {
		int h = rand() % number_of_operators;
		sol = ApplyOperator(sol, h);
		if (m_non_feas.size() == 0) {
			cost = TotalCost(sol); 
			if (cost <= cost_prev) {
				sol_prev = CopyFromTo_sol(sol, sol_prev);
				cost_prev = cost;
				if (cost <= best_cost) {
					best_sol = CopyFromTo_sol(sol, best_sol);
					best_cost = cost;
				}
			}
			else {
				//accepting worse cost
				int delta = cost - cost_prev;
				float randNum = static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
				if (randNum <= exp((float)(-delta) / temp)) {
					//handling solutions
					sol_prev = CopyFromTo_sol(sol, sol_prev);
					cost_prev = cost;
					//temp = temp * temp_modifier;
					temp = temp * (1 - rate_of_cooling);
				}
			}
			m_non_feasp = CopyFromTo_map(m_non_feas, m_non_feasp);
		}
		else {
			sol = CopyFromTo_sol(sol_prev, sol);
			m_non_feas = CopyFromTo_map(m_non_feasp, m_non_feas);
		}
	}
	return best_sol;
}

void OutputFile(vector<vector<int>> initial_sol, vector<vector<int>> sol,
	const char* output_file_name) {
	ofstream output(output_file_name);

	output << "Total cost of initial solution : " << endl;
	output << TotalCost(initial_sol) << endl << endl;

	output << "Total cost of improved solution : " << endl;
	output << TotalCost(sol) << endl << endl;

	//print the solution cost
	/*output << "The improved solution costs :" << endl;
	for (int k = 0; k < number_of_areas; k++) {
		output << "cost of " << airport_in_area[sol[0][k]][sol[1][k]]
			<< "--> " << airport_in_area[sol[0][k + 1]][sol[1][k + 1]]
			<< " = " << cost[getAirportIndex_from_airport(airport_in_area[sol[0][k]]
				[sol[1][k]])]
			[getAirportIndex_from_airport(airport_in_area[sol[0][k + 1]]
				[sol[1][k + 1]])][k]
			<< endl;
	}*/
}
/*
1.in?—?1396
6.in?—?2159
11.in?—?44153
14.in?—?118811
*/

int main() {
	start = clock();
	srand(100);
	number_of_operators = 4;
	ReadFile("../Input/1.in");
	totalTime = 3;
	vector<vector<int>> initial_solution = InitialiseSol();
	vector<vector<int>> improved_solution = ImproveInitialSolution(initial_solution);
	OutputFile(initial_solution, improved_solution,
		"../output/result.out");
	cout << "Elapsed Time = " << (clock() - start) / CLOCKS_PER_SEC << "s\n";
	system("pause");
	return 0;

}

