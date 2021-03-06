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
 * @file exampleIntegralInvariantCurvature2D.cpp
 * @ingroup Examples
 * @author Jérémy Levallois (\c jeremy.levallois@liris.cnrs.fr )
 * Laboratoire d'InfoRmatique en Image et Systèmes d'information - LIRIS (CNRS, UMR 5205), INSA-Lyon, France
 * LAboratoire de MAthématiques - LAMA (CNRS, UMR 5127), Université de Savoie, France
 *
 * @date 2012/12/17
 *
 * An example file named exampleIntegralInvariantCurvature2D.
 *
 * This file is part of the DGtal library.
 */

///////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "DGtal/base/Common.h"

// Shape construction
#include "DGtal/shapes/parametric/Flower2D.h"
#include "DGtal/shapes/GaussDigitizer.h"
#include "DGtal/topology/LightImplicitDigitalSurface.h"
#include "DGtal/topology/DigitalSurface.h"
#include "DGtal/graph/DepthFirstVisitor.h"
#include "DGtal/graph/GraphVisitorRange.h"

// Integral Invariant includes
#include "DGtal/images/ImageHelper.h"
#include "DGtal/geometry/surfaces/FunctorOnCells.h"
#include "DGtal/geometry/surfaces/estimation/IntegralInvariantMeanCurvatureEstimator.h"

// Drawing
#include "DGtal/io/boards/Board2D.h"
#include "DGtal/io/colormaps/GradientColorMap.h"

///////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace DGtal;

///////////////////////////////////////////////////////////////////////////////

int main( int argc, char** argv )
{
    trace.beginBlock ( "Example IntegralInvariantCurvature2D" );
    trace.info() << "Args:";
    for ( int i = 0; i < argc; ++i )
        trace.info() << " " << argv[ i ];
    trace.info() << endl;

    /// Construction of the shape + digitalization
    double h = 0.5;

    typedef Flower2D< Z2i::Space > MyShape;
    typedef GaussDigitizer< Z2i::Space, MyShape > MyGaussDigitizer;
    typedef Z2i::KSpace::Surfel Surfel;
    typedef Z2i::KSpace::SCell SCell;
    typedef LightImplicitDigitalSurface< Z2i::KSpace, MyGaussDigitizer > LightImplicitDigSurface;
    typedef DigitalSurface< LightImplicitDigSurface > MyDigitalSurface;

    MyShape shape( 0, 0, 20.00000124, 10.0000123, 4, 3.0 );

    MyGaussDigitizer digShape;
    digShape.attach( shape );
    digShape.init( shape.getLowerBound(), shape.getUpperBound(), h );
    Z2i::Domain domainShape = digShape.getDomain();
    Z2i::KSpace KSpaceShape;
    bool space_ok = KSpaceShape.init( domainShape.lowerBound(), domainShape.upperBound(), true );
    if ( !space_ok )
    {
        trace.error() << "Error in the Khamisky space construction." << std::endl;
        return 2;
    }

    typedef ImageSelector< Z2i::Domain, unsigned int >::Type Image;
    Image image( domainShape );
    DGtal::imageFromRangeAndValue( domainShape.begin(), domainShape.end(), image );

    SurfelAdjacency<Z2i::KSpace::dimension> SAdj( true );
    Surfel bel = Surfaces<Z2i::KSpace>::findABel( KSpaceShape, digShape, 100000 );
    LightImplicitDigSurface LightImplDigSurf( KSpaceShape, digShape, SAdj, bel );
    MyDigitalSurface digSurf( LightImplDigSurf );

    typedef DepthFirstVisitor< MyDigitalSurface > Visitor;
    typedef GraphVisitorRange< Visitor > VisitorRange;
    typedef VisitorRange::ConstIterator SurfelConstIterator;

    VisitorRange range( new Visitor( digSurf, *digSurf.begin() ) );
    SurfelConstIterator abegin = range.begin();
    SurfelConstIterator aend = range.end();

    typedef ImageToConstantFunctor< Image, MyGaussDigitizer > MyPointFunctor;
    MyPointFunctor pointFunctor( &image, &digShape, 1 );

    /// Integral Invariant stuff
    //! [IntegralInvariantUsage]
    double re_convolution_kernel = 4.5; // Euclidean radius of the convolution kernel. Set by user.

    typedef FunctorOnCells< MyPointFunctor, Z2i::KSpace > MyCellFunctor;
    typedef IntegralInvariantMeanCurvatureEstimator< Z2i::KSpace, MyCellFunctor > MyCurvatureEstimator; // mean curvature estimator

    MyCellFunctor functor ( pointFunctor, KSpaceShape ); // Creation of a functor on Cells, returning true if the cell is inside the shape
    MyCurvatureEstimator estimator ( KSpaceShape, functor );
    estimator.init( h, re_convolution_kernel ); // Initialisation for a grid step h and a given Euclidean radius of convolution kernel re
    std::vector< double > results;
    back_insert_iterator< std::vector< double > > resultsIterator( results ); // output iterator for results of Integral Invariant curvature computation
    estimator.eval ( abegin, aend, resultsIterator ); // Computation of the estimator between two iterators of surfels on the digitized shape border
    //! [IntegralInvariantUsage]

    /// Drawing results
    typedef MyCurvatureEstimator::Quantity Quantity;
    Quantity min = numeric_limits < Quantity >::max();
    Quantity max = numeric_limits < Quantity >::min();
    for ( unsigned int i = 0; i < results.size(); ++i )
    {
        if ( results[ i ] < min )
        {
            min = results[ i ];
        }
        else if ( results[ i ] > max )
        {
            max = results[ i ];
        }
    }
    Board2D board;
    VisitorRange range2( new Visitor( digSurf, *digSurf.begin() ) );
    abegin = range2.begin();

    typedef GradientColorMap< Quantity > Gradient;
    Gradient cmap_grad( min, max );
    cmap_grad.addColor( Color( 50, 50, 255 ) );
    cmap_grad.addColor( Color( 255, 0, 0 ) );
    cmap_grad.addColor( Color( 255, 255, 10 ) );

    board << SetMode( (*abegin).className(), "Paving" );
    string specificStyle = (*abegin).className() + "/Paving";
    for ( unsigned int i = 0; i < results.size(); ++i )
    {
        SCell currentCell = KSpaceShape.sIndirectIncident( *abegin, *KSpaceShape.sOrthDirs( *abegin ) ); // We apply the color to the inner spel (more visible than surfel)
        board << CustomStyle( specificStyle, new CustomColors( Color::Black, cmap_grad( results[ i ] )))
              << currentCell;
        ++abegin;
    }
    board.saveSVG ( "example-integralinvariant2D.svg" );
    trace.endBlock();
    return 0;
}
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
