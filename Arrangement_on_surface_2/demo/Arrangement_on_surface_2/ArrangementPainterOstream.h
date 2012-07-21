#ifndef CGAL_QT_ARRANGEMENT_PAINTER_OSTREAM_H
#define CGAL_QT_ARRANGEMENT_PAINTER_OSTREAM_H
#include <QRectF>

#include <CGAL/Kernel/global_functions.h> // TODO: should be included in PainterOstream.h
#include <CGAL/Qt/PainterOstream.h>
#include <CGAL/Qt/Converter.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_polyline_traits_2.h>
#include <CGAL/Arr_conic_traits_2.h>
#include <CGAL/Arr_linear_traits_2.h>

#include "Utils.h"
#include <vector>

class QPainter;

namespace CGAL {
namespace Qt {

template < class ArrTraits >
class ArrangementPainterOstreamBase
{
public: // typedefs
    typedef ArrTraits Traits;
    typedef typename ArrTraitsAdaptor< Traits >::Kernel Kernel;
    typedef typename Kernel::Point_2 Point_2;
    typedef typename Kernel::Segment_2 Segment_2;
    typedef typename Kernel::Ray_2 Ray_2;
    typedef typename Kernel::Line_2 Line_2;
    typedef typename Kernel::Triangle_2 Triangle_2;
    typedef typename Kernel::Iso_rectangle_2 Iso_rectangle_2;
    typedef typename Kernel::Circle_2 Circle_2;

public: // constructors
    ArrangementPainterOstreamBase( QPainter* p, QRectF clippingRectangle = QRectF( ) ):
        painterOstream( p, clippingRectangle ),
        qp( p ),
        convert( clippingRectangle ),
        scene( NULL ),
        clippingRect( QRectF( ) ), // null rectangle
        scale( 1.0 )
    {
        if ( p != 0 )
        {
            this->scale = p->worldTransform( ).m11( );
        }
    }

public: // methods
    template < class T >
    ArrangementPainterOstreamBase& operator<<( const T& t )
    {
        this->painterOstream << t;
        return *this;
    }

    void setScene( QGraphicsScene* scene_ )
    {
        this->scene = scene_;

        // set the clipping rectangle
        if ( scene_ == NULL )
        {
            return;
        }
        this->clippingRect = this->getViewportRect( );
    }

protected: // methods
    QRectF getViewportRect( ) const
    {
        // assumes scene is not null and attached to exactly one view
        QGraphicsView* view = this->scene->views( ).first( );
        QPointF p1 = view->mapToScene( 0, 0 );
        QPointF p2 = view->mapToScene( view->width( ), view->height( ) );
        QRectF clipRect = QRectF( p1, p2 );

        return clipRect;
    }

protected: // fields
    PainterOstream< Kernel > painterOstream;
    QPainter* qp;
    Converter< Kernel > convert;
    QGraphicsScene* scene;
    QRectF clippingRect;
    double scale;

}; // class ArrangementPainterOstreamBase

template < class ArrTraits >
class ArrangementPainterOstream:
    public ArrangementPainterOstreamBase< ArrTraits >
{
public:
    ArrangementPainterOstream( QPainter* p, QRectF clippingRectangle = QRectF( ) ):
        ArrangementPainterOstreamBase< ArrTraits >( p, clippingRectangle )
    { }
};

template < class Kernel_ >
class ArrangementPainterOstream< CGAL::Arr_segment_traits_2< Kernel_ > >:
    public ArrangementPainterOstreamBase< CGAL::Arr_segment_traits_2< Kernel_ > >
{
public: // typedefs
    typedef Kernel_ Kernel;
    typedef CGAL::Arr_segment_traits_2< Kernel > Traits;
    typedef ArrangementPainterOstreamBase< Traits > Superclass;
    typedef typename Superclass::Point_2 Point_2;
    typedef typename Superclass::Segment_2 Segment_2;
    typedef typename Superclass::Ray_2 Ray_2;
    typedef typename Superclass::Line_2 Line_2;
    typedef typename Superclass::Triangle_2 Triangle_2;
    typedef typename Superclass::Iso_rectangle_2 Iso_rectangle_2;
    typedef typename Superclass::Circle_2 Circle_2;
    typedef typename Traits::Curve_2 Curve_2;
    typedef typename Traits::X_monotone_curve_2 X_monotone_curve_2;

public: // constructors
    ArrangementPainterOstream( QPainter* p, QRectF clippingRectangle = QRectF( ) ):
        Superclass( p, clippingRectangle )
    { }

public: // methods
    ArrangementPainterOstream& operator<<( const X_monotone_curve_2& curve )
    {
        const Point_2& p1 = curve.source( );
        const Point_2& p2 = curve.target( );
        Segment_2 seg( p1, p2 );

        // skip segments outside our view
        QRectF seg_bb = this->convert( seg.bbox( ) );
        if ( this->clippingRect.isValid( ) &&
            ! this->clippingRect.intersects( seg_bb ) )
        {
            return *this;
        }

        this->painterOstream << seg;
        return *this;
    }

    ArrangementPainterOstream& operator<<( const Point_2& p )
    {
        QPointF qpt = this->convert( p );
        // clip the point if possible
        if ( this->clippingRect.isValid( ) &&
            ! this->clippingRect.contains( qpt ) )
        {
            return *this;
        }

        QPen savePen = this->qp->pen( );
        this->qp->setBrush( QBrush( savePen.color( ) ) );
        double radius = savePen.width( ) / 2.0;
        radius /= this->scale;

        this->qp->drawEllipse( qpt, radius, radius );

        this->qp->setBrush( QBrush( ) );
        this->qp->setPen( savePen );
        return *this;
    }

    template < class T >
    ArrangementPainterOstream& operator<<( const T& p )
    {
        (*(static_cast< Superclass* >(this)) << p);
        return *this;
    }
};

template < class SegmentTraits >
class ArrangementPainterOstream< CGAL::Arr_polyline_traits_2< SegmentTraits > > :
    public ArrangementPainterOstreamBase< CGAL::Arr_polyline_traits_2< SegmentTraits > >
{
public: // typedefs
    typedef ArrangementPainterOstreamBase< CGAL::Arr_polyline_traits_2< SegmentTraits > > Superclass;
    typedef typename Superclass::Traits Traits;
    typedef typename Superclass::Kernel Kernel;
    typedef typename Superclass::Point_2 Point_2;
    typedef typename Superclass::Segment_2 Segment_2;
    typedef typename Superclass::Ray_2 Ray_2;
    typedef typename Superclass::Line_2 Line_2;
    typedef typename Superclass::Triangle_2 Triangle_2;
    typedef typename Superclass::Iso_rectangle_2 Iso_rectangle_2;
    typedef typename Superclass::Circle_2 Circle_2;
    typedef typename Traits::Curve_2 Curve_2;
    typedef typename Traits::X_monotone_curve_2 X_monotone_curve_2;

public: // constructors
    ArrangementPainterOstream( QPainter* p, QRectF clippingRectangle = QRectF( ) ):
        Superclass( p, clippingRectangle )
    { }

public: // methods
    ArrangementPainterOstream& operator<<( const X_monotone_curve_2& curve )
    {
        for ( int i = 0; i < curve.size( ); ++i )
        {
            Segment_2 segment = curve[ i ];
            this->painterOstream << segment;
        }
        // TODO: implement polyline painting
#if 0
        const Point_2& p1 = curve.source( );
        const Point_2& p2 = curve.target( );
        Segment_2 seg( p1, p2 );
        this->painterOstream << seg;
#endif
        return *this;
    }

    // cloned from segtraits painter
    ArrangementPainterOstream& operator<<( const Point_2& p )
    {
        QPointF qpt = this->convert( p );
        QPen savePen = this->qp->pen( );
        this->qp->setBrush( QBrush( savePen.color( ) ) );
        double radius = savePen.width( ) / 2.0;
        radius /= this->scale;

        this->qp->drawEllipse( qpt, radius, radius );

        this->qp->setBrush( QBrush( ) );
        this->qp->setPen( savePen );
        return *this;
    }

    template < class T >
    ArrangementPainterOstream& operator<<( const T& p )
    {
        (*(static_cast< Superclass* >(this)) << p);
        return *this;
    }
};

template < class RatKernel, class AlgKernel, class NtTraits >
class ArrangementPainterOstream< CGAL::Arr_conic_traits_2< RatKernel, AlgKernel, NtTraits > >:
    public ArrangementPainterOstreamBase< CGAL::Arr_conic_traits_2< RatKernel, AlgKernel, NtTraits > >
{
public: // typedefs
    typedef CGAL::Arr_conic_traits_2< RatKernel, AlgKernel, NtTraits > Traits;
    typedef ArrangementPainterOstreamBase< Traits > Superclass;
    typedef typename Superclass::Point_2 Point_2;
    typedef typename Superclass::Segment_2 Segment_2;
    typedef typename Superclass::Ray_2 Ray_2;
    typedef typename Superclass::Line_2 Line_2;
    typedef typename Superclass::Triangle_2 Triangle_2;
    typedef typename Superclass::Iso_rectangle_2 Iso_rectangle_2;
    typedef typename Superclass::Circle_2 Circle_2;
    typedef typename Traits::Curve_2 Curve_2;
    typedef typename Traits::X_monotone_curve_2 X_monotone_curve_2;
    typedef typename Traits::Construct_x_monotone_curve_2 Construct_x_monotone_curve_2;
    typedef typename Traits::Point_2 Intersection_point_2;
    typedef typename Traits::Intersect_2 Intersect_2;
    typedef typename Traits::Multiplicity Multiplicity;
    typedef typename ArrTraitsAdaptor< Traits >::Kernel Kernel;
    typedef typename Kernel::FT FT;

public: // inner classes
    // utility class to use with std::sort on an Intersect_2 result set.
    class Compare_intersection_point_result
    {
    public:
        typedef std::pair< Intersection_point_2, Multiplicity > Result;
        // returns whether the point1 < point2, using x-coord to compare
        bool operator()( const Result& o1, const Result& o2 )
        {
            Point_2 p1 = o1.first;
            Point_2 p2 = o2.first;
            return ( p1.x( ) < p2.x( ) );
        }
    };

public: // constructors
    ArrangementPainterOstream( QPainter* p, QRectF clippingRectangle = QRectF( ) ):
        Superclass( p, clippingRectangle ),
        //intersect_2( this->traits.intersect_2_object( ) ), // Why doesn't this work?
        construct_x_monotone_curve_2( this->traits.construct_x_monotone_curve_2_object( ) )
    { }

public: // methods
    ArrangementPainterOstream& operator<<( const X_monotone_curve_2& curve )
    {
        CGAL::Bbox_2 bb = curve.bbox( );
        QRectF qbb = this->convert( bb );
        // quick cull
        if ( this->clippingRect.isValid( ) &&
            ! this->clippingRect.intersects( qbb ) )
        {
            //std::cout << "quick culled curve" << std::endl;
            return *this;
        }

#if 0
        std::cout << "bottom: ("
            << this->clippingRect.bottomLeft( ).x( )
            << " "
            << this->clippingRect.bottomLeft( ).y( )
            << " "
            << this->clippingRect.bottomRight( ).x( )
            << " "
            << this->clippingRect.bottomRight( ).y( )
            << ")"
            << std::endl;
#endif

        if ( this->clippingRect.isValid( ) )
        {
            std::vector< X_monotone_curve_2 > visibleParts;
            if ( this->clippingRect.contains( qbb ) )
            {
                visibleParts.push_back( curve );
            }
            else
            {
                visibleParts = this->visibleParts( curve );
            }
            for ( int i = 0; i < visibleParts.size( ); ++i )
            {
                X_monotone_curve_2 subcurve = visibleParts[ i ];
                int n;
                if ( this->scene == NULL )
                    n = 100; // TODO: get an adaptive approximation
                else
                {
                    QGraphicsView* view = this->scene->views( ).first( );
                    int xmin, xmax;
                    xmin = view->mapFromScene( bb.xmin( ), bb.ymin( ) ).x( );
                    xmax = view->mapFromScene( bb.xmax( ), bb.ymin( ) ).x( );
                    n = xmax - xmin;
                }
                if ( n == 0 )
                {
                    return *this;
                }

                std::pair< double, double >* app_pts = new std::pair< double, double >[ n + 1 ];
                std::pair< double, double >* end_pts = subcurve.polyline_approximation( n, app_pts );
                std::pair< double, double >* p_curr = app_pts;
                std::pair< double, double >* p_next = p_curr + 1;
                int count = 0;
                do
                {
                    QPointF p1( p_curr->first, p_curr->second );
                    QPointF p2( p_next->first, p_next->second );
#if 0
                    Segment_2 seg( p1, p2 );
                    this->painterOstream << seg;
#endif
                    this->qp->drawLine( p1, p2 );
                    p_curr++;
                    p_next++;
                    ++count;
                }
                while ( p_next != end_pts );
            }
        }
        else
        { // draw the whole curve
            int n;
            if ( this->scene == NULL )
                n = 100; // TODO: get an adaptive approximation
            else
            {
                QGraphicsView* view = this->scene->views( ).first( );
                int xmin, xmax;
                xmin = view->mapFromScene( bb.xmin( ), bb.ymin( ) ).x( );
                xmax = view->mapFromScene( bb.xmax( ), bb.ymin( ) ).x( );
                n = xmax - xmin;
            }
            if ( n == 0 )
            {
                return *this;
            }

            std::pair< double, double >* app_pts = new std::pair< double, double >[ n + 1 ];
            std::pair< double, double >* end_pts = curve.polyline_approximation( n, app_pts );
            std::pair< double, double >* p_curr = app_pts;
            std::pair< double, double >* p_next = p_curr + 1;
            int count = 0;
            do
            {
                QPointF p1( p_curr->first, p_curr->second );
                QPointF p2( p_next->first, p_next->second );
#if 0
                Segment_2 seg( p1, p2 );
                this->painterOstream << seg;
#endif
                this->qp->drawLine( p1, p2 );
                p_curr++;
                p_next++;
                ++count;
            }
            while ( p_next != end_pts );
            //std::cout << count << " approximation points" << std::endl;
        }

        return *this;
    }

    // cloned from segtraits painter
    ArrangementPainterOstream& operator<<( const Point_2& p )
    {
        QPointF qpt = this->convert( p );
        QPen savePen = this->qp->pen( );
        this->qp->setBrush( QBrush( savePen.color( ) ) );
        double radius = savePen.width( ) / 2.0;
        radius /= this->scale;

        this->qp->drawEllipse( qpt, radius, radius );

        this->qp->setBrush( QBrush( ) );
        this->qp->setPen( savePen );
        return *this;
    }

    template < class T >
    ArrangementPainterOstream& operator<<( const T& p )
    {
        (*(static_cast< Superclass* >(this)) << p);
        return *this;
    }

protected: // methods
    // Returns subcurves of curve that are actually visible in the view.
    // Assumes that clippingRect is valid.
    std::vector< X_monotone_curve_2 > visibleParts( X_monotone_curve_2 curve )
    {
        // see if we intersect the bottom edge of the viewport
        Intersect_2 intersect_2 = this->traits.intersect_2_object( );
        Point_2 bottomLeft = this->convert( this->clippingRect.bottomLeft( ) );
        Point_2 bottomRight = this->convert( this->clippingRect.bottomRight( ) );
        Point_2 topLeft = this->convert( this->clippingRect.topLeft( ) );
        Point_2 topRight = this->convert( this->clippingRect.topRight( ) );
        X_monotone_curve_2 bottom = 
            this->construct_x_monotone_curve_2( bottomLeft, bottomRight );
        X_monotone_curve_2 left = 
            this->construct_x_monotone_curve_2( bottomLeft, topLeft );
        X_monotone_curve_2 top = 
            this->construct_x_monotone_curve_2( topLeft, topRight );
        X_monotone_curve_2 right = 
            this->construct_x_monotone_curve_2( topRight, bottomRight );

        std::vector< CGAL::Object > bottomIntersections;
        std::vector< CGAL::Object > leftIntersections;
        std::vector< CGAL::Object > topIntersections;
        std::vector< CGAL::Object > rightIntersections;
        std::vector< CGAL::Object > intersections;

        intersect_2( bottom, curve, std::back_inserter( bottomIntersections ) );
        intersect_2( left, curve, std::back_inserter( leftIntersections ) );
        intersect_2( top, curve, std::back_inserter( topIntersections ) );
        intersect_2( right, curve, std::back_inserter( rightIntersections ) );
        int total = bottomIntersections.size( )
            + leftIntersections.size( )
            + topIntersections.size( )
            + rightIntersections.size( );

        intersect_2( bottom, curve, std::back_inserter( intersections ) );
        intersect_2( left, curve, std::back_inserter( intersections ) );
        intersect_2( top, curve, std::back_inserter( intersections ) );
        intersect_2( right, curve, std::back_inserter( intersections ) );

        this->filterIntersectionPoints( intersections );
        //std::cout << "total intersections: " << intersections.size( ) << std::endl;
        //this->printIntersectResult( intersections );

        Point_2 leftEndpt = curve.source( );
        Point_2 rightEndpt = curve.target( );
        if ( leftEndpt.x( ) > rightEndpt.x( ) )
        {
            std::swap( leftEndpt, rightEndpt );
        }
        QPointF qendpt1 = this->convert( leftEndpt );
        QPointF qendpt2 = this->convert( rightEndpt );
        std::list< Point_2 > pointList;
        for ( int i = 0; i < intersections.size( ); ++i )
        {
            CGAL::Object o = intersections[ i ];
            std::pair< Intersection_point_2, Multiplicity > pair;
            if ( CGAL::assign( pair, o ) )
            {
                Point_2 pt = pair.first;
                pointList.push_back( pt );
            }
        }
        bool includeLeftEndpoint = this->clippingRect.contains( qendpt1 );
        bool includeRightEndpoint = this->clippingRect.contains( qendpt2 );
        if ( includeLeftEndpoint )
            pointList.push_front( leftEndpt );
        if ( includeRightEndpoint )
            pointList.push_back( rightEndpt );

        Construct_x_monotone_subcurve_2< Traits > construct_x_monotone_subcurve_2;
        std::vector< X_monotone_curve_2 > clippings;
        typename std::list< Point_2 >::iterator pointListItr = pointList.begin( );
        for ( int i = 0; i < pointList.size( ); i += 2 )
        {
            Point_2 p1 = *pointListItr++;
            Point_2 p2 = *pointListItr++;
            X_monotone_curve_2 subcurve = construct_x_monotone_subcurve_2( curve, p1, p2 );
            clippings.push_back( subcurve );
        }

#if 0
        std::cout << "pointList size: " << pointList.size( ) << std::endl;
        if ( intersections.size( ) % 2 == 0 )
        { // either both curve endpoints are in view or both are out
            if ( this->clippingRect.contains( qendpt1 ) )
            {
                if ( this->clippingRect.contains( qendpt2 ) )
                {
                    std::cout << "both endpoints are in view" << std::endl;
                }
            }
            else if ( !this->clippingRect.contains( qendpt2 ) )
            {
                std::cout << "both endpoints are out of view" << std::endl;
            }
        }
        else
        { // one curve endpoint is in view
            if ( this->clippingRect.contains( qendpt1 ) )
            {
                std::cout << "left endpoint is in view" << std::endl;
            }
            else if ( this->clippingRect.contains( qendpt2 ) )
            {
                std::cout << "right endpoint is in view" << std::endl;
            }
        }

        std::vector< X_monotone_curve_2 > res;
        res.push_back( curve );
        return res;
#endif
        return clippings;
    }

    // keep only the intersection points ie. throw out overlapping curve segments
    void filterIntersectionPoints( std::vector< CGAL::Object >& res )
    {
        std::vector< std::pair< Intersection_point_2, Multiplicity > > tmp;

        // filter out the non-intersection point results
        for ( int i = 0; i < res.size( ); ++i )
        {
            CGAL::Object obj = res[ i ];
            std::pair< Intersection_point_2, Multiplicity > pair;
            if ( CGAL::assign( pair, obj ) )
            {
                tmp.push_back( pair );
            }
        }
        res.clear( );

        // sort the intersection points by x-coord
        Compare_intersection_point_result compare_intersection_point_result;
        std::sort( tmp.begin( ), tmp.end( ), compare_intersection_point_result );

        // box up the sorted elements
        for ( int i = 0; i < tmp.size( ); ++i )
        {
            std::pair< Intersection_point_2, Multiplicity > pair = tmp[ i ];
            CGAL::Object o = CGAL::make_object( pair );
            res.push_back( o );
        }
    }

    void printIntersectResult( const std::vector< CGAL::Object >& res )
    {
        for ( std::vector< CGAL::Object >::const_iterator it = res.begin( );
            it != res.end( ); ++it )
        {
            CGAL::Object obj = *it;
            std::pair< Intersection_point_2, Multiplicity > pair;
            if ( CGAL::assign( pair, obj ) )
            {
                Point_2 pt = pair.first;
                QPointF qpt = this->convert( pt );
                std::cout << "("
                    << pt.x( )
                    << " "
                    << pt.y( )
                    << ")"
                    << std::endl;
            }
        }
    }

protected: // members
    Traits traits;
    //Intersect_2 intersect_2;
    Construct_x_monotone_curve_2 construct_x_monotone_curve_2;
};

template < class Kernel_ >
class ArrangementPainterOstream< CGAL::Arr_linear_traits_2< Kernel_ > >:
    public ArrangementPainterOstreamBase< CGAL::Arr_linear_traits_2< Kernel_ > >
{
public: // typedefs
    typedef Kernel_ Kernel;
    typedef CGAL::Arr_linear_traits_2< Kernel > Traits;
    typedef ArrangementPainterOstreamBase< Traits > Superclass;
    typedef typename Superclass::Point_2 Point_2;
    typedef typename Superclass::Segment_2 Segment_2;
    typedef typename Superclass::Ray_2 Ray_2;
    typedef typename Superclass::Line_2 Line_2;
    typedef typename Superclass::Triangle_2 Triangle_2;
    typedef typename Superclass::Iso_rectangle_2 Iso_rectangle_2;
    typedef typename Superclass::Circle_2 Circle_2;
    typedef typename Traits::Curve_2 Curve_2;
    typedef typename Traits::X_monotone_curve_2 X_monotone_curve_2;

public: // constructors
    ArrangementPainterOstream( QPainter* p, QRectF clippingRectangle = QRectF( ) ):
        Superclass( p, clippingRectangle )
    { }

public: // methods
    ArrangementPainterOstream& operator<<( const X_monotone_curve_2& curve )
    {
        if ( curve.is_segment( ) )
        {
            Segment_2 seg = curve.segment( );

            // skip segments outside our view
            QRectF seg_bb = this->convert( seg.bbox( ) );
            if ( this->clippingRect.isValid( ) &&
                ! this->clippingRect.intersects( seg_bb ) )
            {
                return *this;
            }

            this->painterOstream << seg;
        }
        else if ( curve.is_ray( ) )
        {
            Ray_2 ray = curve.ray( );
            QLineF qseg = this->convert( ray );
            if ( qseg.isNull( ) )
            { // it's out of view
                return *this;
            }
            Segment_2 seg = this->convert( qseg );
            this-> painterOstream << seg;
        }
        else // curve.is_line( )
        {
            Line_2 line = curve.line( );
            QLineF qseg = this->convert( line );
            if ( qseg.isNull( ) )
            { // it's out of view
                return *this;
            }
            Segment_2 seg = this->convert( qseg );
            this-> painterOstream << seg;
        }
        return *this;
    }

    ArrangementPainterOstream& operator<<( const Point_2& p )
    {
        QPointF qpt = this->convert( p );
        // clip the point if possible
        if ( this->clippingRect.isValid( ) &&
            ! this->clippingRect.contains( qpt ) )
        {
            return *this;
        }

        QPen savePen = this->qp->pen( );
        this->qp->setBrush( QBrush( savePen.color( ) ) );
        double radius = savePen.width( ) / 2.0;
        radius /= this->scale;

        this->qp->drawEllipse( qpt, radius, radius );

        this->qp->setBrush( QBrush( ) );
        this->qp->setPen( savePen );
        return *this;
    }

    template < class T >
    ArrangementPainterOstream& operator<<( const T& p )
    {
        (*(static_cast< Superclass* >(this)) << p);
        return *this;
    }
};

} // namespace Qt
} // namespace CGAL
#endif // CGAL_QT_ARRANGEMENT_PAINTER_OSTREAM_H
