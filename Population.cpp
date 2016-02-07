#include "Population.hh"

Population::Population(Problem *problem) : 
        _winner(NULL),
        _problem(problem),
        selections({
            {"fitness-proportional", &Population::fitnessProportionateSelection},
            {"tournament", &Population::tournamentSelection}})
{
    generate();
}

bool Population::solve() {
    unsigned int generation;
    Chromosome *solution = NULL;
    for (generation = 0; generation < config.simulationNumber; ++generation)
    {
        if ((solution = test()) != NULL)
            break;
        reproduce();
    }
    if (solution)
    {
        std::cout << "Solution found in " << generation + 1 << " generation(s)" << std::endl;
        _problem->print(solution->getStrand());
        return true;
    }
    std::cout <<  "Solution not found" << std::endl;
    print();
    return false;
}

void Population::generate()
{
    _problem->askParameters();
    for (unsigned int i = 0; i < config.populationSize; i++)
        _population.push_back(new Chromosome());
}

Chromosome *Population::test()
{
    double fitness;
    _totalFitness = 0;
    // compute fitness of all chromosome
    for (auto &candidate : _population) {
        fitness = _problem->computeFitnessOf(candidate->getStrand());
        candidate->setFitness(fitness);
        _totalFitness += fitness;
    }
    // sort them by fitness
    sortByFitness();
    // test if the best candidate solution solves the problem
    if (_problem->test(_population.front()->getStrand()))
        return _population.front();
    return NULL;
}

void Population::print() const
{
    for (auto &candidate : _population)
        _problem->print(candidate->getStrand());
}

Chromosome *Population::selectChromosome() const
{
    return (this->*selections.at(config.selectionType))();
}

void Population::sortByFitness() {
    std::sort(_population.begin(), _population.end(), Chromosome());
}

void Population::reproduce()
{
  Generation nextGeneration;
  Chromosome *c1, *c2;
  Chromosome::Children children;
  if (config.useElitism == true) {
      for (unsigned int i = 0; i < config.eliteNumber && i < _population.size(); i++)
          nextGeneration.push_back(new Chromosome(_population.at(i)->getStrand()));
  }
  do
  {
    c1 = selectChromosome();
    c2 = selectChromosome();
    if ((double)rand() / RAND_MAX <= config.crossoverRate)
        children = Chromosome::crossover(config.crossoverType, c1, c2);
    else {
	children.first = new Chromosome(c1->getStrand());
	children.second = new Chromosome(c2->getStrand());
    }
    children.first->mutate();
    children.second->mutate();
    nextGeneration.push_back(children.first);
    nextGeneration.push_back(children.second);
  } while (nextGeneration.size() < config.populationSize);
  clean();
  _population = nextGeneration;
}

void Population::clean()
{
    for (Chromosome *p : _population)
        delete p;
}

Population::~Population()
{
  clean();
}

/**
 ** Selections
 */

Chromosome *Population::fitnessProportionateSelection() const
{
    double randomNb = std::fmod(std::rand(), _totalFitness);
    double curFitness = 0;
    for (auto &candidate : _population)
    {
        curFitness += candidate->getFitness();
        if (curFitness >= randomNb)
            return candidate;
    }
    // never happens, just here for the compiler
    return NULL;
}

Chromosome *Population::tournamentSelection() const
{
    const int ts = 5;
    std::vector<int> subPop;
    int id;
    double tmpFitness;
    do {
        do {
            id = rand() % _population.size();
        } while (std::find(subPop.begin(), subPop.end(), id) != subPop.end());
        subPop.push_back(id);
    } while (subPop.size() != ts);
    double f = 0;
    for (auto i : subPop)
    {
        tmpFitness = _population.at(i)->getFitness(); 
        if (tmpFitness > f)
        {
            f = tmpFitness;
            id = i;
        }
    }
    return _population[id];
}
