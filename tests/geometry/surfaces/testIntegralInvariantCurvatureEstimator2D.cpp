/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

/**
 * @file testIntegralInvariantCurvatureEstimator2D.cpp
 * @ingroup Tests
 * @author Jérémy Levallois (\c jeremy.levallois@liris.cnrs.fr )
 * Laboratoire d'InfoRmatique en Image et Systèmes d'information - LIRIS (CNRS, UMR 5205), INSA-Lyon, France
 * LAboratoire de MAthématiques - LAMA (CNRS, UMR 5127), Université de Savoie, France
 *
 * @date 2012/11/28
 *
 * Functions for testing class IntegralInvariantCurvatureEstimator2D.
 *
 * This file is part of the DGtal library.
 */

///////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "DGtal/base/Common.h"

#include "DGtal/shapes/GaussDigitizer.h"
#include "DGtal/topology/LightImplicitDigitalSurface.h"
#include "DGtal/topology/DigitalSurface.h"
#include "DGtal/geometry/surfaces/FunctorOnCells.h"
#include "DGtal/images/ImageHelper.h"
#include "DGtal/graph/DepthFirstVisitor.h"
#include "DGtal/graph/GraphVisitorRange.h"
#include "DGtal/geometry/surfaces/estimation/IntegralInvariantMeanCurvatureEstimator.h"
#include "DGtal/kernel/BasicPointFunctors.h"
#include "DGtal/shapes/implicit/ImplicitBall.h"

///////////////////////////////////////////////////////////////////////////////


using namespace DGtal;

///////////////////////////////////////////////////////////////////////////////
// Functions for testing class IntegralInvariantCurvatureEstimator2D.
///////////////////////////////////////////////////////////////////////////////
/**
 * Example of a test. To be completed.
 *
 */
bool testIntegralInvariantCurvatureEstimator2D ( double h, double delta )
{
  typedef Z2i::Space::RealPoint RealPoint;
  typedef Z2i::KSpace::Surfel Surfel;
  typedef Z2i::Domain Domain;
  typedef ImplicitBall<Z2i::Space> ImplicitShape;
  typedef GaussDigitizer<Z2i::Space, ImplicitShape> DigitalShape;
  typedef LightImplicitDigitalSurface<Z2i::KSpace,DigitalShape> Boundary;
  typedef DigitalSurface< Boundary > MyDigitalSurface;
  typedef DepthFirstVisitor< MyDigitalSurface > Visitor;
  typedef GraphVisitorRange< Visitor > VisitorRange;
  typedef VisitorRange::ConstIterator VisitorConstIterator;
  typedef PointFunctorFromPointPredicateAndDomain< DigitalShape, Z2i::Domain, unsigned int > MyPointFunctor;
  typedef FunctorOnCells< MyPointFunctor, Z2i::KSpace > MySpelFunctor;
  typedef IntegralInvariantMeanCurvatureEstimator< Z2i::KSpace, MySpelFunctor > MyIIMeanEstimator;
  typedef MyIIMeanEstimator::Quantity Quantity;

  double re = 10;
  double radius = 15;
  double realValue = 1.0/radius;

  trace.beginBlock ( "Initialisation of shape ..." );

  ImplicitShape ishape( RealPoint( 0, 0 ), radius );
  DigitalShape dshape;
  dshape.attach( ishape );
  dshape.init( RealPoint( -20.0, -20.0 ), RealPoint( 20.0, 20.0 ), h );

  Z2i::KSpace K;
  if ( !K.init( dshape.getLowerBound(), dshape.getUpperBound(), true ) )
  {
    trace.error() << "Problem with Khalimsky space" << std::endl;
    return false;
  }

  Surfel bel = Surfaces<Z2i::KSpace>::findABel( K, dshape, 10000 );
  Boundary boundary( K, dshape, SurfelAdjacency<Z2i::KSpace::dimension>( true ), bel );
  MyDigitalSurface surf ( boundary );

  trace.endBlock();

  trace.beginBlock( "Initialisation of estimator ..." );

  Domain domain = dshape.getDomain();
  MyPointFunctor pointFunctor( dshape, domain, 1, 0 );
  MySpelFunctor functor( pointFunctor, K );

  MyIIMeanEstimator estimator( K, functor );
  estimator.init( h, re );

  trace.endBlock();

  trace.beginBlock( "Eval estimator" );

  std::vector< Quantity > results;
  std::back_insert_iterator< std::vector< Quantity > > resultsIt( results );

  VisitorRange range( new Visitor( surf, *surf.begin() ));
  VisitorConstIterator ibegin = range.begin();
  VisitorConstIterator iend = range.end();

  estimator.eval( ibegin, iend, resultsIt );

  trace.endBlock();

  trace.beginBlock ( "Comparing results of integral invariant 2D curvature ..." );

  double mean = 0.0;
  double rsize = static_cast<double>(results.size());

  if( rsize == 0 )
  {
    trace.error() << "ERROR: surface is empty" << std::endl;
    trace.endBlock();
    return false;
  }

  for ( unsigned int i = 0; i < rsize; ++i )
  {
    mean += results[ i ];
  }
  mean /= rsize;

  if( mean != mean ) //NaN
  {
    trace.error() << "ERROR: result is NaN" << std::endl;
    trace.endBlock();
    return false;
  }

  double v = std::abs ( realValue - mean );

  trace.warning() << "True value: " << realValue << std::endl;
  trace.warning() << "Mean value: " << mean << std::endl;
  trace.warning() << "Delta: " << delta << " |true - mean|: " << v << std::endl;

  if( v > delta )
  {
    trace.endBlock();
    return false;
  }
  trace.endBlock();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Standard services - public :

int main( int argc, char** argv )
{
  trace.beginBlock ( "Testing class IntegralInvariantCurvatureEstimator2D" );
  trace.info() << "Args:";
  for ( int i = 0; i < argc; ++i )
    trace.info() << " " << argv[ i ];
  trace.info() << std::endl;

  bool res = testIntegralInvariantCurvatureEstimator2D( 0.05, 0.002 ); // && ... other tests
  trace.emphase() << ( res ? "Passed." : "Error." ) << std::endl;
  trace.endBlock();
  return res ? 0 : 1;
}
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
