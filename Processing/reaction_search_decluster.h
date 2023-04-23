#ifndef REACTIONSEARCHDECLUSTER_H
#define REACTIONSEARCHDECLUSTER_H

#include "../DataModel/element.h"
#include "../DataModel/abstract_object.h"

class StructureInfo
{
public:
    StructureInfo(string structure, double k_factor, double bias)
    {
        // Use k_factor and bias to compute a mass.
        m_structure = structure;
        m_slope = k_factor;
        m_bias = bias;
    }

public:
    double calculate(double mass)
    {
        // The formula is y = m_slope * x + m_bias
        // The input of this function is x and it calculates y.
        // Use the input mass and the formulate (m_slope * mass + m_bias) to calculate a new mass
        m_mass = m_slope * mass + m_bias;
        return m_mass;
    }

    double invCalculate(double mass)
    {
        // The formula is y = m_slope * x + m_bias
        // The input of this function is y and it calculates x.
        return (mass - m_bias) / m_slope;
    }

    string getStructure()
    {
        return m_structure;
    }

private:
    string m_structure;
    double m_slope;
    double m_bias;

private:
    double m_mass;
};

struct IsotopePair
{
    Element ele1;
    bool indicator1;
    Element ele2;
    bool indicator2;

    IsotopePair()
    {

    }

    IsotopePair(Element element1, Element element2, bool is1Isotope = false, bool is2Isotope = false)
    {
        ele1 = element1;
        ele2 = element2;
        indicator1 = is1Isotope;
        indicator2 = is2Isotope;
    }

    double getMinRt()
    {
        return ele1.peakParameters.rtmed <= ele2.peakParameters.rtmed ? ele1.peakParameters.rtmed : ele2.peakParameters.rtmed;
    }

    double getMaxRt()
    {
        return ele1.peakParameters.rtmed >= ele2.peakParameters.rtmed ? ele1.peakParameters.rtmed : ele2.peakParameters.rtmed;
    }

    double calculateAverageRt()
    {
        return (ele1.peakParameters.rtmed + ele2.peakParameters.rtmed) / 2;
    }
};

class ReactionSearchDecluster
{
public:
    typedef AbstractObject<Element, IsotopePair> IsotopePairObject;

    struct IsotopePairObjectRTComparator
    {
        bool operator() (IsotopePairObject obj1, IsotopePairObject obj2)
        {
            PairPeakingParameters info1 = obj1.obj.peakParameters;
            PairPeakingParameters info2 = obj2.obj.peakParameters;
            return info1.rtmed < info2.rtmed;
        }
    };

    struct ElementPeakingInfoComparator
    {
        bool operator ()(Element ele1, Element ele2)
        {
            PairPeakingParameters info1 = ele1.peakParameters;
            PairPeakingParameters info2 = ele2.peakParameters;
            if (info1.mzmed < info2.mzmed)
                return true;
            else if (info1.mzmed > info2.mzmed)
                return false;
            else
            {
                if (info1.rtmed < info2.rtmed)
                    return true;
                else if (info1.rtmed > info2.rtmed)
                    return false;
                else
                    return info1.mean1 < info2.mean1;
            }
        }
    };

    struct IsotopePairObjectMassComparator
        {
            bool operator() (IsotopePairObject obj1, IsotopePairObject obj2)
            {
                PairPeakingParameters info1 = obj1.obj.peakParameters;
                PairPeakingParameters info2 = obj2.obj.peakParameters;
                if (info1.mzmed < info2.mzmed)
                    return true;
                else if (info1.mzmed > info2.mzmed)
                    return false;
                else
                {
                    if (info1.rtmed < info2.rtmed)
                        return true;
                    else if (info1.rtmed > info2.rtmed)
                        return false;
                    else
                        return info1.mean1 < info2.mean1;
                }
            }
        };

public:
    ReactionSearchDecluster(
        vector<Element> data,
        const vector<Element>& database,
        PairPeakingParameters pairPeakingParameters,
        ReactionSearchParameters reactionSearchParameters,
        double adduct_massAccuracy,
        vector<Element> data_positive,
        vector<Element> data_negative,
        vector<Element> data_adduct
    );

private:
    void constructPeakingInfo(vector<Element>& data);

    void isotopePairPeaking(
        vector<Element> data,
        PairPeakingParameters thresholds,
        vector<IsotopePair>& pairedData
    );

    void constructAdductList(
        vector<pair<string, double>>& positive,
        vector<pair<string, double>>& negative
    );

    void reactionDatabaseSearch(
        vector<IsotopePair> groups,
        vector<Element> database,
        ReactionSearchParameters reactionSearchParameters,
        vector<pair<string, double>> adductList,
        vector<vector<vector<vector<pair<int, Element>>>>>& hits,
        vector<vector<vector<double>>>& accuracyValues
    );

    bool combinationMassComparison(
        vector<Element> database,
        int maxNumElements,
        double mo,
        double m_prime,
        double del,
        double threshold,
        vector<vector<pair<int, Element>>>& result,
        vector<double>& massAccuracyValues
    );

    bool combinationMassComparison(
        vector<Element> data,
        vector<int> max_appearance,
        int num,
        double del,
        double threshold,
        vector<pair<int, Element>> searched,
        vector<vector<pair<int, Element>>>& result,
        vector<double>& massAccuracyValues
    );

    bool MassFilter(double observed, double theoratical, double threshold);

    double ComputeMass(vector<pair<int, Element>> combination);

    double ComputeMassAccuracy(double observed, double theoratical);

    void retentionTimeBasedGrouping(
        vector<IsotopePair> data,
        double t_rtmed,
        vector<vector<IsotopePair>>& groups
    );

    void constructStructInfo(vector<Element> data, vector<StructureInfo>& structureInfo);
    void generateAdductList(
        vector<Element> data,
        vector<StructureInfo>& adductList
    );

    void adductDeclustering(
        vector<vector<IsotopePair>> groups,
        vector<StructureInfo> adductList,
        vector<StructureInfo> positive,
        vector<StructureInfo> negative,
        bool isPositive,
        double adduct_massAccuracy,
        vector<vector<vector<IsotopePairObject>>>& finalObjects,
        vector<vector<vector<string>>>& finalFormulas
    );

    void isotopePairLabelling(
        vector<IsotopePair>& data,
        vector<IsotopePairObject>& objects
    );

    void groupIsotopeObjectsByMassDifference(
        vector<IsotopePairObject> objects,
        vector<vector<IsotopePairObject>>& groups,
        bool sortByMass
    );

    void sortByPeakingInfo(vector<Element>& data);

    bool sortDatabseByMass(vector<Element>& database);
};

#endif // REACTIONSEARCHDECLUSTER_H
