/*************************************************************************
 *
 * This file is part of the SAMRAI distribution.  For full copyright
 * information, see COPYRIGHT and COPYING.LESSER.
 *
 * Copyright:     (c) 1997-2012 Lawrence Livermore National Security, LLC
 * Description:   Strategy interface to user routines for refining AMR data.
 *
 ************************************************************************/

#ifndef included_xfer_RefinePatchStrategy
#define included_xfer_RefinePatchStrategy

#include "SAMRAI/SAMRAI_config.h"
#include "SAMRAI/hier/Box.h"
#include "SAMRAI/hier/IntVector.h"
#include "SAMRAI/hier/Patch.h"
#include "SAMRAI/hier/PatchLevel.h"
#include "SAMRAI/tbox/Utilities.h"

#include <set>

namespace SAMRAI {
namespace xfer {

/*!
 * @brief Abstract base class for user-defined patch data refining operations
 * and physical boundary filling operations.
 *
 * RefinePatchStrategy provides an interface for a user to supply methods
 * for application-specific refining of data between levels in an
 * AMR patch hierarchy and filling of physical boundary values.
 * A concrete subclass must define four member functions to perform
 * the following tasks:
 *
 * <ul>
 *    <li> define maximum stencil width for user-defined refine operations
 *    <li> fill physical boundary conditions
 *    <li> preprocess the interpolation
 *    <li> postprocess the interpolation
 * </ul>
 *
 * Note that the preprocess member function is called before standard data
 * refine using RefineOperators and the postprocessor member function is called
 * afterwards.
 *
 * There are two versions of the preprocess and postprocess functions.  The
 * default abstract function only takes a single box.  The user may also
 * over-ride preprocess and postprocess functions that take a box list and
 * process an entire patch at one time.  By default, the box list version
 * loops over all of the boxes in the box list and calls the single box version.
 *
 * @see xfer::RefineAlgorithm
 * @see xfer::RefineSchedule
 */

class RefinePatchStrategy
{
public:
   /*!
    * @brief Get the maximum stencil width over all RefinePatchStrategy objects
    * used in an application.
    *
    * @return The maximum of the return values of calls to
    * getRefineOpStencilWidth() for every RefinePatchStrategy of the
    * given Dimension used in an application.
    *
    * @param[in] dim   Only objects with this dimension will be used to
    *                  calculate the max.  If a RefinePatchStrategy with
    *                  another dimension is registered, it will be ignored.
    */
   static hier::IntVector
   getMaxRefineOpStencilWidth(
      const tbox::Dimension& dim);

   /*!
    * @brief Constructor.
    *
    * The constructor will register the constructed object with a static
    * set that manages all RefinePatchStrategy objects in an application.
    */
   explicit RefinePatchStrategy(
      const tbox::Dimension& dim);

   /*!
    * @brief Destructor
    */
   virtual ~RefinePatchStrategy();

   /*!
    * @brief Pure virtual function interface for physical boundary filling.
    *
    * Set data in ghost regions at patch boundaries that touch the
    * physical domain boundary.  The specific data values set in physical
    * boundary ghost regions are determined by the boundary conditions needed
    * by the user application.  The patch data components that should be set
    * in this function correspond to the "scratch" components specified in calls
    * to the registerRefine() function in the RefineAlgorithm class.
    *
    * @param[out] patch               Patch on which to fill boundary data.
    * @param[in] fill_time            Simulation time for boundary filling.
    * @param[in] ghost_width_to_fill  Maximum ghost width to fill over
    *                                 all registered scratch components.
    */
   virtual void
   setPhysicalBoundaryConditions(
      hier::Patch& patch,
      const double fill_time,
      const hier::IntVector& ghost_width_to_fill) = 0;

   /*!
    * @brief Set the ghost data at a multiblock singularity.
    *
    * This virtual method allows for a user-defined implemenation to fill
    * ghost data at ghost regions located at reduced or enhanced connectivity
    * multiblock singularities.  The method is virtual so that it need not
    * be overridden in single-block applications.  The encon_level and
    * dst_to_encon arguments may be ignored if the patch touches
    * no enhanced connectivity singularities.
    *
    * The patches in encon level are in the coordinate system of the blocks
    * where they originated, not in that of the destination patch, so the
    * filling operation must take into account the transformation between
    * blocks.
    *
    * @param patch The patch containing the data to be filled
    * @param encon_level  Level representing enhanced connectivity ghost
    *                     regions
    * @param dst_to_encon  Connector from destination level to encon_level
    * @param fill_time Simulation time at which data is filled
    * @param fill_box Box covering maximum amount of ghost cells to be filled
    * @param boundary_box BoundaryBox describing location of singularity in
    *                     relation to patch
    * @param[in] grid_geometry
    */
   virtual void
   fillSingularityBoundaryConditions(
      hier::Patch& patch,
      const hier::PatchLevel& encon_level,
      const hier::Connector& dst_to_encon,
      const double fill_time,
      const hier::Box& fill_box,
      const hier::BoundaryBox& boundary_box,
      const boost::shared_ptr<hier::BaseGridGeometry>& grid_geometry);

   /*!
    * @brief Return maximum stencil width needed for user-defined
    * data refinement operations performed by this object.
    *
    * This is needed to determine the correct interpolatin data dependencies
    * and to ensure that the data has a sufficient amount of ghost width.
    *
    * For any user-defined interpolation operations implemented in the
    * preprocess or postprocess methods, return the maximum stencil needed
    * on a coarse patch to refine data to a fine patch.
    */
   virtual hier::IntVector
   getRefineOpStencilWidth() const = 0;

   /*!
    * @brief Perform user-defined patch data refinement operations.
    *
    * This member function is called before standard refine operations
    * (expressed using concrete subclasses of the RefineOperator base class).
    * The preprocess function must refine data from the scratch components
    * on the coarse patch into the scratch components of the fine patch
    * on the specified fine box region.  Recall that the scratch components
    * are specified in calls to the registerRefine() function in the
    * RefineAlgorithm class.
    *
    * @param[out] fine     Fine patch containing destination data.
    * @param[in] coarse    Coarse patch containing source data.
    * @param[in] fine_box  Box region on fine patch into which data is refined.
    * @param[in] ratio     Refinement ratio between coarse and fine patches.
    */
   virtual void
   preprocessRefine(
      hier::Patch& fine,
      const hier::Patch& coarse,
      const hier::Box& fine_box,
      const hier::IntVector& ratio) = 0;

   /*!
    * @brief Perform user-defined patch data refinement operations.
    *
    * This member function is called after standard refine operations
    * (expressed using concrete subclasses of the RefineOperator base class).
    * The preprocess function must refine data from the scratch components
    * on the coarse patch into the scratch components of the fine patch
    * on the specified fine box region.  Recall that the scratch components
    * are specified in calls to the registerRefine() function in the
    * RefineAlgorithm class.
    *
    * @param[out] fine     Fine patch containing destination data.
    * @param[in] coarse    Coarse patch containing source data.
    * @param[in] fine_box  Box region on fine patch into which data is refined.     * @param[in] ratio     Refinement ratio between coarse and fine patches.
    */
   virtual void
   postprocessRefine(
      hier::Patch& fine,
      const hier::Patch& coarse,
      const hier::Box& fine_box,
      const hier::IntVector& ratio) = 0;

   /*!
    * Perform user-defined patch data refinement operations on a list of boxes.
    *
    * This member function is called before standard refining operations
    * (expressed using concrete subclasses of the RefineOperator base class).
    *
    * The default implementation of this virtual function loops over the
    * box list and calls the preprocessRefine() method for a single box.
    *
    * @param[out] fine   Fine patch containing destination data.
    * @param[in] coarse  Coarse patch containing source data.
    * @param[in] fine_boxes  List of box regions on fine patch into which data
    *                        is refined.
    * @param[in] ratio     Refinement ratio between coarse and fine patches.
    */
   virtual void
   preprocessRefineBoxes(
      hier::Patch& fine,
      const hier::Patch& coarse,
      const hier::BoxContainer& fine_boxes,
      const hier::IntVector& ratio)
   {
      TBOX_DIM_ASSERT_CHECK_ARGS3(fine, coarse, ratio);
      for (hier::BoxContainer::const_iterator b(fine_boxes);
           b != fine_boxes.end(); ++b) {
         preprocessRefine(fine, coarse, *b, ratio);
      }
   }

   /*!
    * Perform user-defined patch data refinement operations on a list of boxes.
    *
    * This member function is called after standard refining operations
    * (expressed using concrete subclasses of the RefineOperator base class).
    *
    * The default implementation of this virtual function loops over the
    * box list and calls the postprocessRefine() method for a single box.
    *
    * @param[out] fine   Fine patch containing destination data.
    * @param[in] coarse  Coarse patch containing source data.
    * @param[in] fine_boxes  List of box regions on fine patch into which data
    *                        is refined.
    * @param[in] ratio     Refinement ratio between coarse and fine patches.
    */
   virtual void
   postprocessRefineBoxes(
      hier::Patch& fine,
      const hier::Patch& coarse,
      const hier::BoxContainer& fine_boxes,
      const hier::IntVector& ratio)
   {
      TBOX_DIM_ASSERT_CHECK_DIM_ARGS3(d_dim, fine, coarse, ratio);
      for (hier::BoxContainer::const_iterator b(fine_boxes);
           b != fine_boxes.end(); ++b) {
         postprocessRefine(fine, coarse, *b, ratio);
      }
   }

   /*!
    * @brief Return the dimension of this object.
    */
   const tbox::Dimension&
   getDim() const
   {
      return d_dim;
   }

protected:
   /*!
    * @brief Dimension of the object.
    */
   const tbox::Dimension d_dim;

private:
   /*!
    * @brief Get the set of RefinePatchStrategy objects that have been
    * registered.
    */
   static std::set<RefinePatchStrategy *>&
   getCurrentObjects()
   {
      static std::set<RefinePatchStrategy *> current_objects;
      return current_objects;
   }

   /*!
    * @brief Register the object with a set of all RefinePatchStrategy
    * objects used in an application.
    */
   void
   registerObject()
   {
      std::set<RefinePatchStrategy *>& current_objects =
         RefinePatchStrategy::getCurrentObjects();
      current_objects.insert(this);
   }

   /*!
    * @brief Unregister the object.
    */
   void
   unregisterObject()
   {
      std::set<RefinePatchStrategy *>& current_objects =
         RefinePatchStrategy::getCurrentObjects();
      current_objects.erase(this);
   }

};

}
}

#endif
