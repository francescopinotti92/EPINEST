//
//  random.cpp
//  CBM
//
//

#include "random.hpp"


//====== DiscreteDistribution ======//

// Default constructor
// N.B. this is not a valid distribution, DO NOT SAMPLE FROM THIS
DiscreteDistribution::DiscreteDistribution(): nPosWeights( 0 ), valid( false ), degenerate( false ), minVal( std::numeric_limits<uint>::max() ), maxVal( std::numeric_limits<uint>::max() ) {
    
}


// Constructor takes a single integer
DiscreteDistribution::DiscreteDistribution( const uint& n ): nPosWeights( n ), valid( true ) {

    assert( n > 0 );
    weightVec = std::vector<double>( n, 1. / n );
    minVal = 0;
    maxVal = n - 1;
    
    degenerate = ( n == 1 ) ? true : false;
    if ( degenerate ) {
        return;
    }
    
    // compute cumulated weights
    double partialSum = 0.;
    for ( uint i = 0; i < n; i++ ) {
        if ( i < maxVal )
            partialSum += weightVec[i];
        else
            partialSum = 1.; // enforces last cumulated weight to be equal to 1
        cumWeightVec[i] = partialSum;
    }
   
}

// Constructor takes a vector of weights
DiscreteDistribution::DiscreteDistribution( const std::vector<double>& weights ): weightVec( weights ), nPosWeights( 0 ), valid( true ) {
    
    cumWeightVec.resize( weightVec.size(), 0. );
    
    minVal = 0;
    maxVal = static_cast<uint>( weightVec.size() - 1 );
    
    if (minVal == maxVal) {
        degenerate = true;
        nPosWeights = 1;
        return;
    }
    else
        degenerate = false;
    
    double wsum = 0.;
    for ( uint i = 0; i < maxVal + 1; i++ ) {
        wsum += weightVec[i];
        if ( weightVec[i] > 0. )
            ++nPosWeights;
    }
    
    assert(wsum > 0.);
    
    // normalize weights to 1.
    if (wsum != 1.) {
        for (double& w: weightVec)
            w /= wsum;
    }
    
    // compute cumulated weights
    double partialSum = 0.;
    for (uint i = 0; i < maxVal + 1; i++) {
        if (i < maxVal)
            partialSum += weightVec[i];
        else
            partialSum = 1.; // enforces last cumulated weight to be equal to 1
        cumWeightVec[i] = partialSum;
    }
}

DiscreteDistribution getInvalidDiscreteDistribution() {
    
    return DiscreteDistribution() ; // this is not suitable for sampling!

}

//====== AliasTable ======//
AliasTable::AliasTable(const std::vector<double>& probs) {
    
    //assert( std::accumulate( probs.begin(), probs.end(), 0. ) == 1.0 );
    
    K = static_cast<uint>( probs.size() );
    q = std::vector<double>(K, 0.);
    J = std::vector<uint>(K, 0);
        
    std::vector<uint> larger;
    std::vector<uint> smaller;

    larger.reserve( uint(K * 0.5) );
    smaller.reserve( uint(K * 0.5) );
    
    for (uint i = 0; i < K; ++i) {
        q[i] = probs[i] * K;
        if (q[i] < 1.)
            smaller.push_back(i);
        else
            larger.push_back(i);
    }
    
    uint small, large;
    
    while ( ( larger.size() > 0 ) and ( smaller.size() > 0 ) ) {
      
        small = smaller.back();
        large = larger.back();
       
        smaller.pop_back();
        larger.pop_back();
       
        J[small] = large;
        q[large] = q[large] + q[small] - 1.;
        
        if ( q[large] < 1. )
            smaller.push_back(large);
        else
            larger.push_back(large);
    }
}

AliasTable::AliasTable() {
    K = 0;
    q = {};
    J = {};
}

void AliasTable::setAliasTable(const std::vector<double>& probs) {
    
    //assert( std::accumulate( probs.begin(), probs.end(), 0. ) == 1.0 );
    
    K = static_cast<uint>( probs.size() );
    q.resize(K, 0.);
    J.resize(K, 0);
    
    std::vector<uint> larger;
    std::vector<uint> smaller;

    larger.reserve( uint(K * 0.5) );
    smaller.reserve( uint(K * 0.5) );
    
    for (uint i = 0; i < K; ++i) {
        q[i] = probs[i] * K;
        if (q[i] < 1.)
            smaller.push_back(i);
        else
            larger.push_back(i);
    }
    
    uint small, large;
    
    while ( ( larger.size() > 0 ) and ( smaller.size() > 0 ) ) {
      
        small = smaller.back();
        large = larger.back();
       
        smaller.pop_back();
        larger.pop_back();
       
        J[small] = large;
        q[large] = q[large] + q[small] - 1.;
        
        if ( q[large] < 1. )
            smaller.push_back(large);
        else
            larger.push_back(large);
    }
}


// Constructor takes a rv CDF to compute discrete weigths over interval
// dx is the width of bin centroids
// This contructor is better suited for discretizing distributions of time to event.

DiscreteDistribution::DiscreteDistribution(double (*cdf) (double), double dx, uint minVal_, uint maxVal_): minVal(minVal_), maxVal(maxVal_), nPosWeights( 0 ), valid( true ) {
    
    if (minVal == maxVal) {
        degenerate = true;
        nPosWeights = 1;
        return;
    }
    else
        degenerate = false;
    
    weightVec = std::vector<double>(maxVal - minVal + 1, 0.);
    cumWeightVec = std::vector<double>(maxVal - minVal + 1, 0.);

    double wsum = 0.;
    double x = minVal;
    for (uint i = 0; i < maxVal - minVal + 1; i++) {
        weightVec[i] = cdf(x + 0.5 * dx) - cdf(x - 0.5 * dx);
        wsum += cdf(x);
        x += dx;
    }
    
    // normalize weights
    for (uint i = 0; i < maxVal - minVal + 1; i++) {
        if ( weightVec[i] > 0. )
            ++nPosWeights;
        weightVec[i] = weightVec[i] / wsum;
    }
    
    // compute cumulated weights
    double partialSum = 0.;
    for (uint i = 0; i < maxVal + 1; i++) {
        if (i < maxVal)
            partialSum += weightVec[i];
        else
            partialSum = 1.; // enforces last cumulated weight to be equal to 1
        cumWeightVec[i] = partialSum;
    }
}

// u is a uniform random number in the range [0,1]
// returns random integer (in [minVal, maxVal]) according;
uint DiscreteDistribution::getVal(const double& u) const {
    
    if ( !valid )
        return std::numeric_limits<uint>::max() ;
    
    if ( degenerate )
        return minVal;
    
    for ( uint i = 0; i < maxVal - minVal + 1; i++ ) {
        if ( u <= cumWeightVec[i] )
            return minVal + i;
    }
    
    return maxVal;
}


//====== Random ======//

// default constructor
// uses std::random_device
Random::Random(): m_mt(std::mt19937(13637) ) {
    m_mt.seed( std::random_device()() );
    rand_uniform.param(std::uniform_real_distribution<double>::param_type(0., 1.) );
    rand_normal.param(std::normal_distribution<double>::param_type(0., 1.) );
    rand_exponential.param(std::exponential_distribution<double>::param_type(1.) );
}

// seed constructor
Random::Random(uint seed): m_mt(std::mt19937(seed) ) {
    rand_uniform.param(std::uniform_real_distribution<double>::param_type(0., 1.) );
    rand_normal.param(std::normal_distribution<double>::param_type(0., 1.) );
    rand_exponential.param(std::exponential_distribution<double>::param_type(1.) );
}

// uses std::random_device
void Random::reseed(uint seed) {
    m_mt = std::mt19937(seed);
}

double Random::getUni() {
    return rand_uniform(m_mt);
}

// uniform rv over ]0,1[ (i.e. extrema excluded)
double Random::getUniPos() {
    double r;
    do
        r = getUni();
    while ( r * ( 1. - r ) == 0 );
    return r;
}

double Random::getNormal(const double& mean, const double& std) {
    if (std == 0.)
        return mean;
    else
        return mean + std * rand_normal(m_mt);
}

double Random::getExpo(const double& rate) {
    return rand_exponential(m_mt) / rate;
}

bool Random::getBool(const double& prob) {
    if (prob == 0.)
        return false;
    else
        return (getUni() <= prob) ? true : false;
}

uint Random::getUniInt(const uint& min, const uint& max) {
    if (min == max)
        return min;
    else
        return min + static_cast<uint>(getUni() * (max - min + 1) );
}

uint Random::getUniInt(const uint& max) {
    if (max == 0)
        return 0;
    else
        return static_cast<uint>( getUni() * (max + 1) );
} // extrema inclusive

// Geometric distribution with parameter p
// p(k) = p(1 - p)^k , k = 0,1,2,...
// N.B. expected value is 1/p - 1, not 1/p
uint Random::getGeom(const double& p) {
    if (p >= 1.)
        return 0;
    return floor( log( 1. - getUni() ) / log(1. - p) ); // 1 - U is never 0
}

// Geometric distribution with parameter p
// p(k) = p(1-p)^{k-1}, k = 1,2,3,...
// expected value is 1/p
uint Random::getGeom1(const double &p) {
    if ( p == 1. )
        return 1;
    return 1 + floor( log( 1. - getUni() ) / log(1. - p) );
}


// a is the shape parameter and b is the scale
double Random::getGamma (const double& a, const double& b)
{
  /* assume a > 0 */
  uint na = floor (a);

  if( a >= std::numeric_limits<uint>::max() )
    {
      return b * ( gamma_large ( floor (a) ) + gamma_frac ( a - floor (a) ) );
    }
  else if (a == na)
    {
      return b * gamma_int (na);
    }
  else if (na == 0)
    {
      return b * gamma_frac (a);
    }
  else
    {
      return b * ( gamma_int (na) + gamma_frac (a - na) ) ;
    }
}

double Random::gamma_int (const uint& a)
{
  if (a < 12)
    {
      uint i;
      double prod = 1;

      for (i = 0; i < a; i++)
        {
          prod *= getUniPos();
        }

      /* Note: for 12 iterations we are safe against underflow, since
         the smallest positive random number is O(2^-32). This means
         the smallest possible product is 2^(-12*32) = 10^-116 which
         is within the range of double precision. */

      return -log (prod);
    }
  else
    {
      return gamma_large ( (double) a );
    }
}


double Random::gamma_large (const double& a)
{
  /* Works only if a > 1, and is most efficient if a is large */

  double sqa, x, y, v;
  sqa = sqrt (2 * a - 1);
  do
    {
      do
        {
          y = tan ( M_PI * getUni() );
          x = sqa * y + a - 1;
        }
      while (x <= 0);
      v = getUni();
    }
  while (v > (1 + y * y) * exp ((a - 1) * log (x / (a - 1)) - sqa * y));

  return x;
}

double Random::gamma_frac (const double& a)
{
  /* This is exercise 16 from Knuth; see page 135, and the solution is
     on page 551.  */

  double p, q, x, u, v;

  if (a == 0) {
    return 0;
  }

  p = M_E / (a + M_E);
  do
    {
      u = getUni();
      v = getUniPos();

      if (u < p)
        {
          x = exp ((1 / a) * log (v));
          q = exp (-x);
        }
      else
        {
          x = 1 - log (v);
          q = exp ((a - 1) * log (x));
        }
    }
  while ( getUni() >= q );

  return x;
}

double Random::getBeta (const double& a, const double& b)
{
  if ( (a <= 1.0) && (b <= 1.0) )
    {
      double U, V, X, Y;
      while (1)
        {
          U = getUniPos();
          V = getUniPos();
          X = pow( U, 1./a );
          Y = pow( V, 1./b );
          if ( (X + Y) <= 1. )
            {
              if (X + Y > 0)
                {
                  return X/ (X + Y);
                }
              else
                {
                  double logX = log(U)/a;
                  double logY = log(V)/b;
                  double logM = logX > logY ? logX: logY;
                  logX -= logM;
                  logY -= logM;
                  return exp(logX - log(exp(logX) + exp(logY)));
                }
            }
        }
    }
  else
    {
      double x1 = getGamma (a, 1.);
      double x2 = getGamma (b, 1.);
      return x1 / (x1 + x2);
    }
}

uint Random::getBinom(double p, uint n) {
    uint i, a, b, k = 0;
    while (n > 10) {      /* This parameter is tunable */
        double X;
        a = 1 + (n / 2);
        b = 1 + n - a;
        
        X = getBeta ( (double) a, (double) b );
        
        if (X >= p) {
            n = a - 1;
            p /= X;
        }
        else {
            k += a;
            n = b - 1;
            p = (p - X) / (1 - X);
        }
    }
    
    for (i = 0; i < n; i++) {
        if ( getBool(p) ) {
            k++;
        }
    }
    return k;
}

uint Random::getPoisson (double mu)
{
  double emu;
  double prod = 1.;
  uint k = 0;

  while (mu > 10)
    {
      uint m = mu * (7. / 8.);

      double X = gamma_int (m);

      if (X >= mu)
        {
          return k + getBinom (mu / X, m - 1);
        }
      else
        {
          k += m;
          mu -= X;
        }
    }

  /* This following method works well when mu is small */

  emu = exp (-mu);

  do
    {
      prod *= getUni();
      k++;
    }
  while (prod > emu);

  return k - 1;

}

// sample from zero-truncated Poisson distribution
// expected value is mu / ( 1 - e^{-mu} )
uint Random::getZeroTruncPoisson( const double &mu ) {
    uint res;
    do {
        res = getPoisson( mu );
    }
    while ( res == 0 );
    return res;
}


// sample from negative binomial distribution
// uses same notation as numpy/scipy (NOT wikipedia)
// expected value is mu = n * ( 1 - p ) / p
// variance is var = mu * ( 1 + mu / n )
uint Random::getNegBinom( double p, const double n ) {

    if ( p == 1. )
        return 0; //

    double X = getGamma( n, 1. ) ;
    uint k = getPoisson( X * ( 1 - p ) / p ) ;
    return k ;
    
}


// sample from discrete distribution (naive method => must provide DiscreteDistribution object)
uint Random::getCustomDiscrete( const DiscreteDistribution& dd ) {
    double u = getUni();
    return dd.getVal(u);
}

// sample from discrete distribution (alias method => must provide alias table)
uint Random::getCustomDiscrete( const AliasTable &dd ) {
    uint kk = getUniInt( dd.K - 1 );
    return ( getBool(dd.q[kk]) ) ? kk : dd.J[kk];
}
