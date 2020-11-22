#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <iostream> 
#include <fstream>
#include <string>
#include <boost/foreach.hpp>

using namespace boost::geometry; 
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
using point = bg::model::point<int, 2, bg::cs::cartesian>;
using linestring = bg::model::linestring<point>;
using polygon = bg::model::polygon<point>;
using box = bg::model::box<point>;

using transl_strans = bg::strategy::transform::translate_transformer<int, 2, 2>;

// create the same poly as provided but with lower left corned of the 
// envellop at the provided position.
auto get_poly_at_position(const polygon& poly, const point& position)
{
        //auto right_size_previous = bg::get<0>(bg::return_envelope<box>(*previous).max_corner());
        point dir = position;
        bg::subtract_point(
                dir,
                bg::return_envelope<box>(poly).min_corner());

        auto inner_translate = transl_strans(bg::get<0>(dir), bg::get<1>(dir));
        auto translated_poly = std::make_shared<polygon>();
        bg::transform(poly, *translated_poly, inner_translate);
         
        return translated_poly;
}

int main()
{
    typedef std::pair<box, std::shared_ptr<polygon>> value;
    bgi::rtree<value, bgi::quadratic<16> > inner_polygons; 

    polygon outer{{point(0,0),point(100,0),point(100,100),point(0,100)}};
    polygon inner_poly_model{{point(0,0),point(10,0),point(10,10),point(0,10)}};

    polygon trans_inner_poly;
    //auto translate = transl_strans(2.0,2.0);
    //boost::geometry::transform(inner_poly, trans_inner_poly, translate);

    // find bouding box of previous poly
    // try to put one on the right 
    //  -> if ok -> accept it and repeat
    //  -> if nok -> start a new row using the bouding box of the first of your current row 
    box outer_box;
    envelope(outer, outer_box);
    auto min_corner_outer = outer_box.min_corner();
    auto max_corner_outer = outer_box.max_corner();

    box inner_model_box;
    envelope(inner_poly_model ,inner_model_box);
    auto min_corner_inner_model = inner_model_box.min_corner();

    //// vector from the bottum left corner of the poly model to the outmodel
    auto first_poly =  get_poly_at_position(inner_poly_model, min_corner_inner_model);
    box box_potential_poly = bg::return_envelope<box>(*first_poly);
    inner_polygons.insert(std::make_pair(box_potential_poly,first_poly));

    // Start from the worst case right side of the previous poly.
    auto previous = first_poly;
    auto previous_head_box = previous;

    auto viable_head_found = true;
    while(viable_head_found){
        auto right_size_previous = bg::get<0>(bg::return_envelope<box>(*previous).max_corner());
        auto potential_inner_poly =  get_poly_at_position(
                inner_poly_model,
                point(right_size_previous, bg::get<1>(outer_box.min_corner())));

        // Check inner of inner_poly (we can have touching poly's) is full inside the inner of the 
        // outer poly.
        // so the external of the outer poly and internal of inner poly shen intersected should never form a plane.
        //
        auto rel = bg::relation(*potential_inner_poly, outer)[6]; 
        if(rel != '2') // no plane...
        {
            inner_polygons.insert(
                    std::make_pair(
                        bg::return_envelope<box>(*potential_inner_poly),
                        potential_inner_poly));
        }
        else{
            // Restart at the left side.
            // Get top left corner of the last head,
            // put lower left corder of inner_poly_model on it...
            
            
            // if we accept it, put it as new head, if no new head was found quit!
            viable_head_found = false;
        }

        previous = potential_inner_poly;
    }

    std::ofstream outputFile;
    outputFile.open("polys.wkt");

    outputFile << bg::wkt<polygon>(outer) << std::endl;
    for(auto p : inner_polygons)
    {
        outputFile << bg::wkt<polygon>(*p.second) << std::endl;
    }
    outputFile.close();

    return 0;
}
