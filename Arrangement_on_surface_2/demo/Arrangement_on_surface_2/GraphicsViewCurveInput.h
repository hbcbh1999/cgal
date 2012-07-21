#ifndef CGAL_QT_GRAPHICS_VIEW_CURVE_INPUT_H
#define CGAL_QT_GRAPHICS_VIEW_CURVE_INPUT_H
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_polyline_traits_2.h>
#include <CGAL/Arr_conic_traits_2.h>
#include <CGAL/Arr_linear_traits_2.h>
#include <CGAL/Qt/GraphicsViewInput.h>
#include <CGAL/Qt/Converter.h>
#include <QEvent>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <iostream>
#include "Callback.h"
#include "ISnappable.h"
#include <CGAL/CORE_algebraic_number_traits.h>

namespace CGAL {
namespace Qt {

class GraphicsViewCurveInputBase:
    public GraphicsViewInput, public ISnappable
{
public:
    virtual void setScene( QGraphicsScene* scene_ );
    virtual QGraphicsScene* getScene( ) const;

    void setSnappingEnabled( bool b );
    void setSnapToGridEnabled( bool b );

protected:
    GraphicsViewCurveInputBase( QObject* parent );
    virtual void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
    virtual void mousePressEvent( QGraphicsSceneMouseEvent* event );
    virtual bool eventFilter( QObject* obj, QEvent* event );

    QRectF viewportRect( ) const;

    QGraphicsScene* scene;
    bool snappingEnabled;
    bool snapToGridEnabled;

}; // class GraphicsViewCurveInputBase

template < class ArrTraits >
class GraphicsViewCurveInput:
    public GraphicsViewCurveInputBase
{ };

/**
Specialization of GraphicsViewCurveInput for Arr_segment_traits_2; handles
user-guided generation of line segment curves.
*/
template < class Kernel_ >
class GraphicsViewCurveInput< CGAL::Arr_segment_traits_2< Kernel_ > >:
    public GraphicsViewCurveInputBase
{
public:
    typedef Kernel_ Kernel;
    typedef CGAL::Arr_segment_traits_2< Kernel > Traits;
    typedef typename Traits::Curve_2 Curve_2;
    typedef typename Kernel::Point_2 Point_2;
    typedef typename Kernel::Segment_2 Segment_2;

    GraphicsViewCurveInput( QObject* parent ):
        GraphicsViewCurveInputBase( parent ),
        second( false )
    { }

protected:
    void mouseMoveEvent( QGraphicsSceneMouseEvent* event )
    {
        if ( this->second )
        {
            Point_2 clickedPoint = this->snapPoint( event );
            Segment_2 segment( this->p1, clickedPoint );
            QLineF qSegment = this->convert( segment );
            this->segmentGuide.setLine( qSegment );
        }
    }

    void mousePressEvent( QGraphicsSceneMouseEvent* event )
    {
        if ( !this->second )
        {
            this->second = true;
            this->p1 = this->snapPoint( event );
            QPointF pt = this->convert( this->p1 );
            this->segmentGuide.setLine( pt.x( ), pt.y( ), pt.x( ), pt.y( ) );
            if ( this->scene != NULL )
            {
                this->scene->addItem( &( this->segmentGuide ) );
            }
        }
        else
        {
            this->second = false;
            this->p2 = this->snapPoint( event );
            if ( this->scene != NULL )
            {
                this->scene->removeItem( &( this->segmentGuide ) );
            }
            Curve_2 res( this->p1, this->p2 );
            emit generate( CGAL::make_object( res ) );
        }
    }

    // override this to snap to the points you like
    virtual Point_2 snapPoint( QGraphicsSceneMouseEvent* event )
    {
        Point_2 clickedPoint = this->convert( event->scenePos( ) );
        return clickedPoint;
    }

    Converter< Kernel > convert;
    Point_2 p1;
    Point_2 p2;
    bool second;

    QGraphicsLineItem segmentGuide;
}; // class GraphicsViewCurveInput< CGAL::Arr_segment_traits_2< Kernel_ > >

/**
Specialization of GraphicsViewCurveInput for Arr_polyline_traits_2; handles
user-guided generation of line segment curves.
*/
template < class SegmentTraits >
class GraphicsViewCurveInput< CGAL::Arr_polyline_traits_2< SegmentTraits > >:
    public GraphicsViewCurveInputBase
{
public:
    typedef CGAL::Arr_polyline_traits_2< SegmentTraits > Traits;
    typedef typename Traits::Curve_2 Curve_2;
    typedef typename SegmentTraits::Kernel Kernel;
    typedef typename Kernel::Point_2 Point_2;
    typedef typename Kernel::Segment_2 Segment_2;

    GraphicsViewCurveInput( QObject* parent ):
        GraphicsViewCurveInputBase( parent )
    { }

protected:
    void mouseMoveEvent( QGraphicsSceneMouseEvent* event )
    {
        if ( ! this->polylineGuide.empty( ) )
        {
            Point_2 clickedPoint = this->snapPoint( event );
            // TODO: make it work for the latest line segment
            Segment_2 segment( this->points.back( ), clickedPoint );
            QLineF qSegment = this->convert( segment );
            this->polylineGuide.back( )->setLine( qSegment );
        }
    }

    void mousePressEvent( QGraphicsSceneMouseEvent* event )
    {
        Point_2 clickedPoint = this->snapPoint( event );
        if ( this->points.empty( ) )
        { // first
            // add clicked point to polyline
            this->points.push_back( clickedPoint );

            QPointF pt = this->convert( clickedPoint );
            QGraphicsLineItem* lineItem = new QGraphicsLineItem( pt.x( ), pt.y( ), pt.x( ), pt.y( ) );
            this->polylineGuide.push_back( lineItem );
            if ( this->scene != NULL )
            {
                this->scene->addItem( this->polylineGuide.back( ) );
            }
        }
        else
        {
            // add clicked point to polyline
            this->points.push_back( clickedPoint );

            if ( event->button( ) == ::Qt::RightButton )
            { // finalize polyline input
                for ( int i = 0; i < this->polylineGuide.size( ); ++i )
                {
                    if ( this->scene != NULL )
                    {
                        this->scene->removeItem( this->polylineGuide[ i ] );
                    }
                    delete this->polylineGuide[ i ];
                }
                this->polylineGuide.clear( );
                Curve_2 res( this->points.begin( ), this->points.end( ) );
                this->points.clear( );

                emit generate( CGAL::make_object( res ) );
            }
            else
            { // start the next segment
                QPointF pt = this->convert( clickedPoint );
                QGraphicsLineItem* lineItem = new QGraphicsLineItem( pt.x( ), pt.y( ), pt.x( ), pt.y( ) );
                this->polylineGuide.push_back( lineItem );
                if ( this->scene != NULL )
                {
                    this->scene->addItem( this->polylineGuide.back( ) );
                }
            }
        }
    }

    // override this to snap to the points you like
    virtual Point_2 snapPoint( QGraphicsSceneMouseEvent* event )
    {
        Point_2 clickedPoint = this->convert( event->scenePos( ) );
        return clickedPoint;
    }

    Converter< Kernel > convert;
    std::vector< Point_2 > points;

    std::vector< QGraphicsLineItem* > polylineGuide;
}; // class GraphicsViewCurveInput< CGAL::Arr_polyline_traits_2< SegmentTraits > >

/**
Specialization of GraphicsViewCurveInput for Arr_conic_traits_2; handles
user-guided generation of conic curves.
*/
template < class RatKernel, class AlgKernel, class NtTraits >
class GraphicsViewCurveInput< CGAL::Arr_conic_traits_2< RatKernel, AlgKernel, NtTraits > >:
    public GraphicsViewCurveInputBase
{
public:
    typedef CGAL::Arr_conic_traits_2< RatKernel, AlgKernel, NtTraits > Traits;
    typedef typename Traits::Curve_2 Curve_2;
    typedef typename Traits::Point_2 Point_2; // basically, an AlgKernel::Point_2 with metadata
    typedef typename Traits::Construct_x_monotone_curve_2 Construct_x_monotone_curve_2;
    typedef AlgKernel Kernel;
    //typedef typename Kernel::Point_2 Point_2;
    typedef typename Kernel::Segment_2 Segment_2;
    typedef typename RatKernel::FT Rat_FT;
    typedef typename RatKernel::Point_2 Rat_point_2;
    typedef typename RatKernel::Segment_2 Rat_segment_2;
    typedef typename RatKernel::Circle_2 Rat_circle_2;
    typedef enum ConicType
    {
        CONIC_SEGMENT,
        CONIC_CIRCLE,
        CONIC_ELLIPSE,
        CONIC_THREE_POINT,
        CONIC_FIVE_POINT
    } ConicType;

    GraphicsViewCurveInput( QObject* parent ):
        GraphicsViewCurveInputBase( parent ),
        construct_x_monotone_curve_2( this->traits.construct_x_monotone_curve_2_object( ) ),
        conicType( CONIC_SEGMENT ),
        circleItem( NULL ),
        ellipseItem( NULL )
    { }

    void setConicType( ConicType conicType_ )
    {
        this->conicType = conicType_;
    }

    ConicType getConicType( ) const
    {
        return this->conicType;
    }

protected:
    void mouseMoveEvent( QGraphicsSceneMouseEvent* event )
    {
        if ( ! this->polylineGuide.empty( ) )
        {
            if ( this->conicType == CONIC_SEGMENT )
            {

                Point_2 clickedPoint = this->snapPoint( event );
                // TODO: make it work for the latest line segment
                Segment_2 segment( this->points.back( ), clickedPoint );
                QLineF qSegment = this->convert( segment );
                this->polylineGuide.back( )->setLine( qSegment );
            }
        }
        if ( this->circleItem != NULL )
        {
            QPointF p1 = this->convert( this->points.back( ) );
            QPointF p2 = this->convert( this->snapPoint( event ) );
            double radius = sqrt( (p1.x( ) - p2.x( ))*(p1.x( ) - p2.x( ))
                + (p1.y( ) - p2.y( ))*(p1.y( ) - p2.y( )) );
            double d = radius * sqrt( 2.0 );
            this->circleItem->setRect( p1.x( ) - radius, p1.y( ) - radius, 2*radius, 2*radius );
        }
        if ( this->ellipseItem != NULL )
        {
            Point_2 p1 = this->points.back( );
            Point_2 p2 = this->snapPoint( event );
            CGAL::Bbox_2 bb = p1.bbox( ) + p2.bbox( );
            double w = bb.xmax( ) - bb.xmin( );
            double h = bb.ymax( ) - bb.ymin( );
            this->ellipseItem->setRect( bb.xmin( ), bb.ymin( ), w, h );
        }
    }

    void mousePressEvent( QGraphicsSceneMouseEvent* event )
    {
        Point_2 clickedPoint = this->snapPoint( event );
        this->points.push_back( clickedPoint );

        if ( this->points.size( ) == 1 )
        { // first
            // add clicked point to polyline

            if ( this->conicType == CONIC_SEGMENT )
            {
                QPointF pt = this->convert( clickedPoint );
                QGraphicsLineItem* lineItem = new QGraphicsLineItem( pt.x( ), pt.y( ), pt.x( ), pt.y( ) );
                this->polylineGuide.push_back( lineItem );
                if ( this->scene != NULL )
                {
                    this->scene->addItem( this->polylineGuide.back( ) );
                }
            }
            else if ( this->conicType == CONIC_CIRCLE )
            {
                QPointF pt = this->convert( clickedPoint );
                if ( this->scene != NULL )
                {
                    QGraphicsEllipseItem* ellipse = this->scene->addEllipse( pt.x( ), pt.y( ), 0, 0 );
                    this->circleItem = ellipse;
                }
            }
            else if ( this->conicType == CONIC_ELLIPSE )
            {
                QPointF pt = this->convert( clickedPoint );
                if ( this->scene != NULL )
                {
                    QGraphicsEllipseItem* ellipse = this->scene->addEllipse( pt.x( ), pt.y( ), 0, 0 );
                    this->ellipseItem = ellipse;
                }
            }
        }
        else
        {
            if ( this->conicType == CONIC_SEGMENT )
            {
                for ( int i = 0; i < this->polylineGuide.size( ); ++i )
                {
                    if ( this->scene != NULL )
                    {
                        this->scene->removeItem( this->polylineGuide[ i ] );
                    }
                    delete this->polylineGuide[ i ];
                }
                this->polylineGuide.clear( );

                //Curve_2 res = this->construct_x_monotone_curve_2( this->points[ 0 ], this->points[ 1 ] );
                double x1 = CGAL::to_double( this->points[ 0 ].x( ) ); 
                double y1 = CGAL::to_double( this->points[ 0 ].y( ) ); 
                double x2 = CGAL::to_double( this->points[ 1 ].x( ) ); 
                double y2 = CGAL::to_double( this->points[ 1 ].y( ) ); 
                Curve_2 res = Curve_2( Rat_segment_2( Rat_point_2( x1, y1 ), Rat_point_2( x2, y2 ) ) );
                //std::cout << "res is " << ( (res.is_valid( ))? "" : "not ") << "valid" << std::endl;
                this->points.clear( );

                emit generate( CGAL::make_object( res ) );
            }
            else if ( this->conicType == CONIC_CIRCLE )
            {
                if ( this->scene != NULL )
                {
                    this->scene->removeItem( this->circleItem );
                }
                this->circleItem = NULL;

                std::cout << "TODO: Add the circle" << std::endl;
                double x1 = CGAL::to_double( this->points[ 0 ].x( ) ); 
                double y1 = CGAL::to_double( this->points[ 0 ].y( ) ); 
                double x2 = CGAL::to_double( this->points[ 1 ].x( ) ); 
                double y2 = CGAL::to_double( this->points[ 1 ].y( ) ); 
                double sq_rad = CGAL::square(x2 - x1) + CGAL::square(y2 - y1);
                Curve_2 res = Curve_2( Rat_circle_2( Rat_point_2( x1, y1 ), sq_rad ) );

                this->points.clear( );
                emit generate( CGAL::make_object( res ) );
            }
            else if ( this->conicType == CONIC_ELLIPSE )
            {
                if ( this->scene != NULL )
                {
                    this->scene->removeItem( this->ellipseItem );
                }
                this->ellipseItem = NULL;

                CGAL::Bbox_2 bb = this->points[ 0 ].bbox( ) + this->points[ 1 ].bbox( );
                double x1 = CGAL::to_double( bb.xmin( ) );
                double y1 = CGAL::to_double( bb.ymin( ) );
                double x2 = CGAL::to_double( bb.xmax( ) );
                double y2 = CGAL::to_double( bb.ymax( ) );
                double sq_rad = CGAL::square(x2 - x1) + CGAL::square(y2 - y1);

                Rat_FT a = CORE::abs( Rat_FT(x1) - Rat_FT(x2) )/2;
                Rat_FT b = CORE::abs( Rat_FT(y1) - Rat_FT(y2) )/2;
                Rat_FT a_sq = a*a;
                Rat_FT b_sq = b*b;
                Rat_FT x0 = (x2 + x1)/2;
                Rat_FT y0 = (y2 + y1)/2;

                Rat_FT r = b_sq;
                Rat_FT s = a_sq;
                Rat_FT t = 0;
                Rat_FT u = -2*x0*b_sq;
                Rat_FT v = -2*y0*a_sq;
                Rat_FT ww = x0*x0*b_sq + y0*y0*a_sq - a_sq*b_sq;

                Curve_2 res = Curve_2( r, s, t, u, v, ww );
                this->points.clear( );
                emit generate( CGAL::make_object( res ) );
            }
            else if ( this->conicType == CONIC_THREE_POINT )
            {
                if ( this->points.size( ) == 3 )
                {
                    QPointF qp1 = this->convert( this->points[ 0 ] );
                    QPointF qp2 = this->convert( this->points[ 1 ] );
                    QPointF qp3 = this->convert( this->points[ 2 ] );
                    Rat_point_2 p1 = Rat_point_2( qp1.x( ), qp1.y( ) );
                    Rat_point_2 p2 = Rat_point_2( qp2.x( ), qp2.y( ) );
                    Rat_point_2 p3 = Rat_point_2( qp3.x( ), qp3.y( ) );
                    RatKernel ker;
                    if ( ! ker.collinear_2_object()( p1, p2, p3 ) )
                    {
                        Curve_2 res( p1, p2, p3 );
                        emit generate( CGAL::make_object( res ) );
                    }
                    else
                    {
                        std::cout << "Oops, points don't specify a valid conic. Try again!" << std::endl;
                    }

                    // TODO: make a valid curve and insert it

                    this->points.clear( );
                }
            }
            else if ( this->conicType == CONIC_FIVE_POINT )
            {
                if ( this->points.size( ) == 5 )
                {
                    QPointF qp1 = this->convert( this->points[ 0 ] );
                    QPointF qp2 = this->convert( this->points[ 1 ] );
                    QPointF qp3 = this->convert( this->points[ 2 ] );
                    QPointF qp4 = this->convert( this->points[ 3 ] );
                    QPointF qp5 = this->convert( this->points[ 4 ] );
                    Rat_point_2 p1 = Rat_point_2( qp1.x( ), qp1.y( ) );
                    Rat_point_2 p2 = Rat_point_2( qp2.x( ), qp2.y( ) );
                    Rat_point_2 p3 = Rat_point_2( qp3.x( ), qp3.y( ) );
                    Rat_point_2 p4 = Rat_point_2( qp4.x( ), qp4.y( ) );
                    Rat_point_2 p5 = Rat_point_2( qp5.x( ), qp5.y( ) );
                    try
                    {
                        Curve_2 res( p1, p2, p3, p4, p5 );
                        if ( res.is_valid( ) )
                        {
                            std::cout << "Meow, valid 5 point conic!" << std::endl;
                            emit generate( CGAL::make_object( res ) );
                        }
                        else
                        {
                            std::cout << "Oops, points don't specify a valid conic. Try again!" << std::endl;
                        }
                        this->points.clear( );

                    } 
                    catch (...)
                    {
                        std::cout << "Oops, points don't specify a valid conic. Try again!" << std::endl;
                        this->points.clear( );
                    }
                }
            }
        }
    }

    // override this to snap to the points you like
    virtual Point_2 snapPoint( QGraphicsSceneMouseEvent* event )
    {
        Point_2 clickedPoint = this->convert( event->scenePos( ) );
        return clickedPoint;
    }

    Converter< Kernel > convert;
    std::vector< Point_2 > points;
    std::vector< QGraphicsLineItem* > polylineGuide;
    QGraphicsEllipseItem* circleItem;
    QGraphicsEllipseItem* ellipseItem;

    Traits traits;
    Construct_x_monotone_curve_2 construct_x_monotone_curve_2;
    ConicType conicType;
}; // class GraphicsViewCurveInput< CGAL::Arr_conic_traits_2< RatKernel, AlgKernel, NtTraits > >

/**
Specialization of GraphicsViewCurveInput for Arr_linear_traits_2; handles
user-guided generation of line segment curves.
*/
template < class Kernel_ >
class GraphicsViewCurveInput< CGAL::Arr_linear_traits_2< Kernel_ > >:
    public GraphicsViewCurveInputBase
{
public: // typedefs
    typedef Kernel_ Kernel;
    typedef GraphicsViewCurveInputBase Superclass;
    typedef CGAL::Arr_linear_traits_2< Kernel > Traits;
    typedef typename Traits::Curve_2 Curve_2;
    typedef typename Kernel::Point_2 Point_2;
    typedef typename Kernel::Segment_2 Segment_2;
    typedef typename Kernel::Ray_2 Ray_2;
    typedef typename Kernel::Line_2 Line_2;
    enum CurveType
    {
        SEGMENT, RAY, LINE
    };

public: // constructors
    GraphicsViewCurveInput( QObject* parent ):
        GraphicsViewCurveInputBase( parent ),
        second( false ),
        curveType( SEGMENT )
    { }

public: // methods
    void setCurveType( CurveType type )
    {
        this->curveType = type;
    }

protected: // methods
    virtual bool eventFilter( QObject* obj, QEvent* event )
    {
        // before we do anything, update the clipping rect
        // TODO: somehow only update this when the view changes
        QRectF clippingRect = this->viewportRect( );
        this->convert = Converter< Kernel >( clippingRect );

        // now handle the event
        return Superclass::eventFilter( obj, event );
    }

    void mouseMoveEvent( QGraphicsSceneMouseEvent* event )
    {
        if ( this->second )
        {
            Point_2 hoverPoint = this->snapPoint( event );
            if ( p1 == hoverPoint )
                return;
            QLineF qSegment;
            if ( this->curveType == SEGMENT )
            {
                Segment_2 segment( this->p1, hoverPoint );
                qSegment = this->convert( segment );
            }
            else if ( this->curveType == RAY )
            {
                Ray_2 ray( this->p1, hoverPoint );
                qSegment = this->convert( ray );
            }
            else // this->curveType == LINE
            {
                Line_2 line( this->p1, hoverPoint );
                qSegment = this->convert( line );
            }
            this->segmentGuide.setLine( qSegment );
        }
    }

    void mousePressEvent( QGraphicsSceneMouseEvent* event )
    {
        if ( !this->second )
        { // fix our first point
            this->second = true;
            this->p1 = this->snapPoint( event );
            QPointF pt = this->convert( this->p1 );
            this->segmentGuide.setLine( pt.x( ), pt.y( ), pt.x( ), pt.y( ) );
            if ( this->scene != NULL )
            {
                this->scene->addItem( &( this->segmentGuide ) );
            }
        }
        else // this->second == true
        {
            this->second = false;
            this->p2 = this->snapPoint( event );

            // skip if degenerate
            if ( this->p1 == this->p2 )
                return;

            if ( this->scene != NULL )
            {
                this->scene->removeItem( &( this->segmentGuide ) );
            }

            Curve_2 res;
            if ( this->curveType == SEGMENT )
            {
                res = Curve_2( Segment_2( this->p1, this->p2 ) );
            }
            else if ( this->curveType == RAY )
            {
                res = Curve_2( Ray_2( this->p1, this->p2 ) );
            }
            else // this->curveType == LINE
            {
                res = Curve_2( Line_2( this->p1, this->p2 ) );
            }

            emit generate( CGAL::make_object( res ) );
        }
    }

    // override this to snap to the points you like
    virtual Point_2 snapPoint( QGraphicsSceneMouseEvent* event )
    {
        Point_2 clickedPoint = this->convert( event->scenePos( ) );
        return clickedPoint;
    }

protected: // fields
    Converter< Kernel > convert;
    Point_2 p1;
    Point_2 p2;
    bool second;

    QGraphicsLineItem segmentGuide;
    CurveType curveType;
}; // class GraphicsViewCurveInput< CGAL::Arr_linear_traits_2< Kernel_ > >

} // namespace Qt
} // namespace CGAL
#endif // CGAL_QT_GRAPHICS_VIEW_SEGMENT_INPUT_H
