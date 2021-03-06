#ifndef PressureSource_Expr_h
#define PressureSource_Expr_h

#include <map>

//-- ExprLib Includes --//
#include <expression/Expression.h>

#include <spatialops/structured/FVStaggered.h>
#include <CCA/Components/Wasatch/VardenParameters.h>
#include <CCA/Components/Wasatch/TimeIntegratorTools.h>
/**
 *  \ingroup WasatchExpressions
 *  \class  PressureSource
 *  \author Amir Biglari
 *  \author Tony Saad
 *  \author James C. Sutherland
 *  \date	July, 2011
 *
 *  \brief Calculates the source term for the pressure equation ( i.e. the second
 *         derivaive of density with respect to time) of the form \f$ \frac{
 *         \nabla.(\rho u)^n + (\alpha (\frac{\partial \rho}{\partial t})^{n+1} 
 *         - (1 - \alpha) \nabla.(\rho u)^{n+1}) }{\Delta t} = \frac{\partial^2
 *         \rho}{\partial t^2} \f$ for variable density low-Mach problems, where \f$\alpha\f$ 
 *         is a weighting factor applied on the continuity equation and added to the 
 *         equations. This requires knowledge of the momentum field and, the velocity  
 *         vactor and the density field in future time steps.
 *         OR
 *         of the form \f$ \frac{ \rho \nabla.u^n }{\Delta t} = \frac{\partial^2
 *         \rho}{\partial t^2} \f$ which requires knowledge of a dilitation field.
 *  
 *  Note that here the dilitation at time step n+1 is enforced to be 0, but at the
 *       current time step is allowed not to be zero. By retaining this term, we can
 *       remain consistent even when initial conditions do not satisfy the governing
 *       equations.
 *
 */
class PressureSource : public Expr::Expression<SVolField>
{  
  
  typedef SpatialOps::FaceTypes<SVolField> FaceTypes;
  typedef FaceTypes::XFace XFace; ///< The type of field for the x-face of SVolField.
  typedef FaceTypes::YFace YFace; ///< The type of field for the y-face of SVolField.
  typedef FaceTypes::ZFace ZFace; ///< The type of field for the z-face of SVolField.

  typedef SpatialOps::SingleValueField TimeField;

  typedef SpatialOps::OperatorTypeBuilder< SpatialOps::Interpolant, SVolField, XVolField >::type S2XInterpOpT;
  typedef SpatialOps::OperatorTypeBuilder< SpatialOps::Interpolant, SVolField, YVolField >::type S2YInterpOpT;
  typedef SpatialOps::OperatorTypeBuilder< SpatialOps::Interpolant, SVolField, ZVolField >::type S2ZInterpOpT;
  
  typedef SpatialOps::OperatorTypeBuilder< SpatialOps::Interpolant, XVolField, SVolField >::type X2SInterpOpT;
  typedef SpatialOps::OperatorTypeBuilder< SpatialOps::Interpolant, YVolField, SVolField >::type Y2SInterpOpT;
  typedef SpatialOps::OperatorTypeBuilder< SpatialOps::Interpolant, ZVolField, SVolField >::type Z2SInterpOpT;

  typedef SpatialOps::OperatorTypeBuilder< SpatialOps::Gradient, XVolField, SVolField >::type GradXT;
  typedef SpatialOps::OperatorTypeBuilder< SpatialOps::Gradient, YVolField, SVolField >::type GradYT;
  typedef SpatialOps::OperatorTypeBuilder< SpatialOps::Gradient, ZVolField, SVolField >::type GradZT;
  
  typedef SpatialOps::OperatorTypeBuilder< SpatialOps::GradientX, SVolField, SVolField >::type GradXST;
  typedef SpatialOps::OperatorTypeBuilder< SpatialOps::GradientY, SVolField, SVolField >::type GradYST;
  typedef SpatialOps::OperatorTypeBuilder< SpatialOps::GradientZ, SVolField, SVolField >::type GradZST;

  const XVolField *xMom_, *xMomOld_, *uStar_, *xVel_;
  const YVolField *yMom_, *yMomOld_, *vStar_, *yVel_;
  const ZVolField *zMom_, *zMomOld_, *wStar_, *zVel_;
  const SVolField *dens_, *densStar_, *dens2Star_, *divmomstar_;
  const SVolField *dil_;
  const TimeField *timestep_, *rkStage_;
  
  const bool isConstDensity_, doX_, doY_, doZ_, is3d_, useOnePredictor_;
  
  const Expr::Tag xMomt_, yMomt_, zMomt_, xMomOldt_, yMomOldt_, zMomOldt_;
  const Expr::Tag xVelt_, yVelt_, zVelt_, xVelStart_, yVelStart_, zVelStart_, denst_, densStart_, dens2Start_, dilt_, timestept_,rkstaget_, divmomstart_;
  
  const Wasatch::TimeIntegrator* timeIntInfo_;

  const double a0_;
  const Wasatch::VarDenParameters::VariableDensityModels model_;
  
  const GradXT* gradXOp_;
  const GradYT* gradYOp_;
  const GradZT* gradZOp_;
  const GradXST* gradXSOp_;
  const GradYST* gradYSOp_;
  const GradZST* gradZSOp_;
  const S2XInterpOpT* s2XInterpOp_;
  const S2YInterpOpT* s2YInterpOp_;
  const S2ZInterpOpT* s2ZInterpOp_;  
  const X2SInterpOpT* x2SInterpOp_;
  const Y2SInterpOpT* y2SInterpOp_;
  const Z2SInterpOpT* z2SInterpOp_;
  
  PressureSource( const Expr::TagList& momTags,
                  const Expr::TagList& oldMomTags,
                  const Expr::TagList& velTags,
                  const Expr::TagList& velStarTags,
                  const bool isConstDensity,
                  const Expr::Tag densTag,
                  const Expr::Tag densStarTag,
                  const Expr::Tag dens2StarTag,
                  const Wasatch::VarDenParameters varDenParams,
                  const Expr::Tag divmomstarTag);
public:
  
  /**
   *  \brief Builder for the source term in the pressure poisson equation 
   *         (i.e. the second derivaive of density with respect to time)
   */
  class Builder : public Expr::ExpressionBuilder
  {
  public:
    
    /**
     *  \brief Constructs a builder for source term of the pressure
     *  \param results the tags for the result fields calculated by this expression.
     *         See the documentation of the PressureSource class for more details.
     *         Order is critically important.
     *  \param momTags a list tag which holds the tags for momentum in
     *         all directions
     *  \param velTags a list tag which holds the tags for velocity at
     *         the current time stage in all directions
     *  \param velStarTags a list tag which holds the tags for velocity at
     *         the time stage "*" in all directions
     *  \param isConstDensity
     *  \param densTag a tag to hold density in constant density cases, which is 
     *         needed to obtain drhodt 
     *  \param densStarTag a tag for estimation of density at the time stage "*"
     *         which is needed to obtain momentum at that stage.
     *  \param dens2StarTag a tag for estimation of density at the time stage "**"
     *         which is needed to calculate drhodt
     *  \param varDenParams
     *  \param divMomStarTag a tag for estimation of divmom at the time stage "*"
     */
    Builder( const Expr::TagList& results,
             const Expr::TagList& momTags,
             const Expr::TagList& oldMomTags,
             const Expr::TagList& velTags,
             const Expr::TagList& velStarTags,
             const bool isConstDensity,
             const Expr::Tag densTag,
             const Expr::Tag densStarTag,
             const Expr::Tag dens2StarTag,
             const Wasatch::VarDenParameters varDenParams,
             const Expr::Tag divMomStarTag);
    
    ~Builder(){}
    
    Expr::ExpressionBase* build() const;
    
  private:
    const bool isConstDens_;
    const Expr::TagList momTs_, oldMomTags_, velTs_, velStarTs_;
    const Expr::Tag denst_, densStart_, dens2Start_, divmomstart_;
    const Wasatch::VarDenParameters varDenParams_;
  };
  
  ~PressureSource();
  void advertise_dependents( Expr::ExprDeps& exprDeps );
  void bind_fields( const Expr::FieldManagerList& fml );
  void bind_operators( const SpatialOps::OperatorDatabase& opDB );
  void evaluate();
  
};

#endif // PressureSource_Expr_h
