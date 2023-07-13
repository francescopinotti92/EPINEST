//
//  random.hpp
//  CBM_ECS
//
//

#ifndef random_h
#define random_h

#include "types.hpp"
#include <math.h>


//====== Class to wrap discrete distributions ======//
// actually sample from this through getCustomDiscrete(const DiscreteDistribution& dd);
class DiscreteDistribution {
public:
    DiscreteDistribution() ;
    DiscreteDistribution( const uint& n ) ;
    DiscreteDistribution( const std::vector<double>& weights ) ;
    DiscreteDistribution( double (*cdf) (double), double dt, uint minVal_, uint maxVal_) ;
    bool isDegenerate() { return degenerate ; }
    bool isValid() { return valid ; }
    uint getVal(const double& u) const ;
    const uint& getNumberPosWeights() { return nPosWeights ; }
private:
    bool valid;
    bool degenerate;
    uint minVal;
    uint maxVal;
    uint nPosWeights;
    std::vector<double> weightVec;
    std::vector<double> cumWeightVec;
};

DiscreteDistribution getInvalidDiscreteDistribution();

//====== Class to sample from a discrete distribution using Alias method ======//
// actually sample from this through getCustomDiscrete(const AliasTable& dd);
struct AliasTable {
    AliasTable(const std::vector<double>& probs);
    AliasTable();
    void setAliasTable(const std::vector<double>& probs);
    std::vector<double> q;  // vector of probabilities (binary mixtures for alias method)
    std::vector<uint>   J;  // vector of aliases
    uint K;                 // number of elements
};

//====== Wraps together a random number generator and functions to sample from several rvs ======//

class Random {
public:
    Random();
    Random(uint seed);
    void reseed(uint seed);
    double getUni();
    double getUniPos();
    double getNormal(const double& mean, const double& std);
    double getExpo(const double& rate = 1.);
    double getGamma (const double& a, const double& b); // a is shape, b is scale (not rate)
    double getBeta(const double& a, const double& b);
        
    uint getUniInt(const uint& min, const uint& max);
    uint getUniInt(const uint& max);
    uint getGeom(const double& p);
    uint getGeom1(const double& p);
    uint getBinom(double p, uint n);
    uint getPoisson(double mu);
    uint getZeroTruncPoisson(const double& mu);
    uint getNegBinom(double p, const double n);
    uint getCustomDiscrete(const DiscreteDistribution& dd);
    uint getCustomDiscrete(const AliasTable& dd);

    
    bool getBool(const double& prob);

    // template methods
    
    // shuffles elements in std::vector
    template<class T>
    void shuffleVector(std::vector<T>& v) { std::shuffle(v.begin(), v.end(), m_mt);}
    
    // get a random element from std::vector
    template<class T>
    T getRandomElemFromVec( const std::vector<T>& v ) {
        assert( v.size() > 0 );
        if ( v.size() == 1 )
            return v[0];
        return v[ this->getUniInt( static_cast<uint>( v.size() - 1 ) ) ];
    }
    
    // removes n random elements from std::vector
    template<class T>
    void removeRandomElementsFromVec( std::vector<T>& v, const uint& n ) {
        if ( n >= static_cast<uint>( v.size() ) ) {
            v.clear(); // remove all elements
        }
        else {
            uint counter = 0;
            uint size = static_cast<uint>( v.size() );
            uint idx = 0;
            while( counter < n ) {
                idx = getUniInt( size - 1 );
                std::swap( v[idx], v.back() );
                v.pop_back();
                --size;
                ++counter;
            }
        }
    }
    

private:
    std::mt19937 m_mt;
    std::uniform_real_distribution<double> rand_uniform;
    std::normal_distribution<double> rand_normal;
    std::exponential_distribution<double> rand_exponential;
    
    double gamma_int (const uint& a);
    double gamma_large (const double& a);
    double gamma_frac (const double& a);
};


#endif /* random_h */
