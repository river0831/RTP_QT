#include "reaction_search_decluster.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>

#include <future>
#include <functional>
#include <ctime>

// Test
#include <QDebug>



using namespace std;

mutex mtx;

/* Constructor */
ReactionSearchDecluster::ReactionSearchDecluster(QObject* parent) : QObject(parent)
{
    success_ = false;
}

void ReactionSearchDecluster::runProcess(
    vector<Element> data,
    const vector<Element>& database,
    PairPeakingParameters pairPeakingParameters,
    ReactionSearchParameters reactionSearchParameters,
    double adduct_massAccuracy,
    vector<Element> data_positive,
    vector<Element> data_negative,
    vector<Element> data_adduct,
    int num_threads
) {
    success_ = false;
    rsd_results_.rt_groups.clear();

    /**************Reaction search**********************/
    // Step 0: Construct peaking info for input data.
    constructPeakingInfo(data);

    // Step 1: Pair peaking: find peak pairs.
    vector<IsotopePair> pairedData;
    PairPeakingParameters peakingThresholds = pairPeakingParameters;
    isotopePairPeaking(data, peakingThresholds, pairedData);

    // Step 2: Construct adduct list
    vector<pair<string, double>> adductList_positive;
    vector<pair<string, double>> adductList_negative;
    constructAdductList(adductList_positive, adductList_negative);
    vector<pair<string, double>> adductList = reactionSearchParameters.isPositive ? adductList_positive : adductList_negative;

    // Step 3: Sort pairs
    vector<IsotopePairObject> pairedObjects;
    for (int i = 0; i < pairedData.size(); i++)
    {
        Element e = pairedData[i].ele1.peakParameters.rtmed < pairedData[i].ele2.peakParameters.rtmed ? pairedData[i].ele1 : pairedData[i].ele2;
        IsotopePairObject object = IsotopePairObject(e, pairedData[i]);
        pairedObjects.push_back(object);
    }
    sort(pairedObjects.begin(), pairedObjects.end(), IsotopePairObjectRTComparator());
    pairedData.clear();
    for (int i = 0; i < pairedObjects.size(); i++)
        pairedData.push_back(pairedObjects[i].attributes);

    // Step 4: reaction database search
    vector<vector<vector<vector<pair<int, Element>>>>> results;
    vector<vector<vector<double>>> massAccuracyValues;
    reactionDatabaseSearch(pairedData, database, reactionSearchParameters, adductList, results, massAccuracyValues, num_threads);

    // Step 4.5: Update results to paired data
    for (int i = 0; i < pairedData.size(); i++)
    {
        vector<ReactionSearchResult> reactionSearchResults;
        vector<vector<vector<pair<int, Element>>>> possibility_allAdduct = results[i];
        for (int j = 0; j < possibility_allAdduct.size(); j++)
        {
            vector<vector<pair<int, Element>>> possibility_oneAdduct = possibility_allAdduct[j];
            vector<double> accuracy_oneAdduct = massAccuracyValues[i][j];
            string type = adductList[j].first;
            for (int k = 0; k < possibility_oneAdduct.size(); k++)
            {
                string combination;
                double sum_of_mass = 0;
                // Compute the 'value' of a ReactionSearchResult and sum of masses
                vector<pair<int, Element>> possibility_oneAdduct_k = possibility_oneAdduct[k];
                vector<pair<int, string>> descriptions(possibility_oneAdduct_k.size());
                for (int m = 0; m < possibility_oneAdduct_k.size(); m++)
                {
                    int no = possibility_oneAdduct_k[m].first;
                    string description = possibility_oneAdduct_k[m].second.getPropertyValue("Description");
                    descriptions[m].first = no;
                    descriptions[m].second = description;
                    if (m != 0)
                        combination = combination + "+";
                    combination = combination + to_string(no) + "*" + "[" + description + "]";
                    sum_of_mass = sum_of_mass + no * stof(possibility_oneAdduct_k[m].second.getPropertyValue("Monoisotopic Mass Change"));
                }
                // Get value
                string value = combination;
                // Get accuracy
                double accuracy = abs(accuracy_oneAdduct[k]);
                // Compute the sum of masses
                double totalMass = sum_of_mass + reactionSearchParameters.m_prime;
                // Construct ReactionSearchResult
                ReactionSearchResult rsr = ReactionSearchResult(type, value, accuracy, totalMass, descriptions);
                reactionSearchResults.push_back(rsr);
            }
        }

        // Update data to elements;
        pairedData[i].ele1.reactionSearchResults = reactionSearchResults;
        pairedData[i].ele2.reactionSearchResults = reactionSearchResults;        
    }

    // Step 4.6: Compute ratio for each pair
    for (int i = 0; i < pairedData.size(); i++)
    {
        double mean1 = stof(pairedData[i].ele1.getPropertyValue("dataset2_mean"));
        double mean2 = stof(pairedData[i].ele2.getPropertyValue("dataset2_mean"));
        double ratio = max(mean1, mean2) / min(mean1, mean2);
        pairedData[i].ele1.addProperty("Ratio", to_string(ratio));
        pairedData[i].ele2.addProperty("Ratio", to_string(ratio));
    }

    /**************Declustering**********************/
    // Step 6: Retention time-based grouping.
    vector<vector<IsotopePair>> isotopePairGroups;
    double t_rtmed = 0.06;
    retentionTimeBasedGrouping(pairedData, t_rtmed, isotopePairGroups);

    // Step 7: Construct 'positive' and 'negative' dictionaries.
    vector<StructureInfo> positive;
    vector<StructureInfo> negative;
    constructStructInfo(data_positive, positive);
    constructStructInfo(data_negative, negative);

    // Step 8: Adduct declustering.
    vector<StructureInfo> adductList_clustering;
    vector<vector<vector<IsotopePairObject>>> finalObjects;
    vector<vector<vector<string>>> finalFormulas;
    generateAdductList(data_adduct, adductList_clustering);

    // Change second parameter from adductList_clustering ->
    if (true)
    {
        adductList_clustering.clear();
        vector<pair<string, double>> adduct_list_toProcess = reactionSearchParameters.isPositive ? adductList_positive : adductList_negative;
        for (int i = 0; i < adduct_list_toProcess.size(); i++)
        {
            StructureInfo info = StructureInfo(adduct_list_toProcess[i].first, 1, adduct_list_toProcess[i].second);
            adductList_clustering.push_back(info);
        }
    }

    adductDeclustering(isotopePairGroups, adductList_clustering, positive, negative, reactionSearchParameters.isPositive, adduct_massAccuracy, finalObjects, finalFormulas);

    // Step 8.5: Add group id and pair id
    for (int i = 0; i < finalObjects.size(); i++)
    {
        // [[pair...], [pair...]]
        int groupID = i + 1;
        string GroupID = to_string(groupID);
        for (int j = 0; j < finalObjects[i].size(); j++)
        {
            // [pair...]
            for (int k = 0; k < finalObjects[i][j].size(); k++)
            {
                int pairID = k;
                string PairID = GroupID + "_" + to_string(pairID);
                Element ele1 = finalObjects[i][j][k].attributes.ele1;
                Element ele2 = finalObjects[i][j][k].attributes.ele2;
                finalObjects[i][j][k].attributes.ele1.addProperty("GroupID", GroupID);
                finalObjects[i][j][k].attributes.ele2.addProperty("GroupID", GroupID);
                finalObjects[i][j][k].attributes.ele1.addProperty("PairID", PairID);
                finalObjects[i][j][k].attributes.ele2.addProperty("PairID", PairID);
            }
        }
    }

    // Step 8.6: Update reaction search result
    for (int i = 0; i < finalObjects.size(); i++)
    {
        for (int j = 0; j < finalObjects[i].size(); j++)
        {
            for (int k = 0; k < finalObjects[i][j].size(); k++)
            {
                Element ele1 = finalObjects[i][j][k].attributes.ele1;
                Element ele2 = finalObjects[i][j][k].attributes.ele2;
                vector<string> formulas;
                for (int m = 0; m < finalFormulas[i].size(); m++)
                {
                    string formula = finalFormulas[i][m][j];
                    if (formula != "")
                        formulas.push_back(formula);
                }

                string reaction_search_result;
                vector<ReactionSearchResult> reactionSearchResults = ele1.reactionSearchResults;
                for (int m = 0; m < reactionSearchResults.size(); m++)
                {
                    // Only add the results that are included in formulas
                    ReactionSearchResult rsr = reactionSearchResults[m];
                    bool inSet = false;
                    for (int n = 0; n < formulas.size(); n++)
                    {
                        string fullType = "M+" + rsr.type;
                        if (formulas[n] == fullType)
                        {
                            inSet = true;
                            break;
                        }
                    }
                    if (formulas.size() == 0)
                        inSet = true;
                    if (inSet)
                    {
                        // Add it to reaction_search_result
                        reaction_search_result = reaction_search_result + "(";
                        reaction_search_result = reaction_search_result + rsr.value + ";";
                        reaction_search_result = reaction_search_result + rsr.type + ";";
                        reaction_search_result = reaction_search_result + to_string(rsr.sum) + ";";
                        reaction_search_result = reaction_search_result + to_string(rsr.accuracy);
                        reaction_search_result = reaction_search_result + ")";
                    }

                }
                finalObjects[i][j][k].attributes.ele1.addProperty("Reaction Search Result", reaction_search_result);
                finalObjects[i][j][k].attributes.ele2.addProperty("Reaction Search Result", reaction_search_result);
            }
        }
    }

    // Step 9: Update formula to elements
    for (int i = 0; i < finalObjects.size(); i++)
    {
        for (int j = 0; j < finalObjects[i].size(); j++)
        {
            for (int k = 0; k < finalObjects[i][j].size(); k++)
            {
                Element ele1 = finalObjects[i][j][k].attributes.ele1;
                Element ele2 = finalObjects[i][j][k].attributes.ele2;
                string propertyName = "formula";
                string formulas;
                for (int m = 0; m < finalFormulas[i].size(); m++)
                {
                    string formula = finalFormulas[i][m][j];
                    formulas = formulas + to_string(m) + "[" + formula + "]" + ";";
                }

                finalObjects[i][j][k].attributes.ele1.addProperty(propertyName, formulas);
                finalObjects[i][j][k].attributes.ele2.addProperty(propertyName, formulas);
            }
        }
    }

    // Save results
    ReactionSearchDeclusterResult rsd_results;
    rsd_results.rt_groups.reserve(finalObjects.size());
    for (int i = 0; i < finalObjects.size(); ++i) {
        // finalObjects[i] represents a RententionTimeGroup
        RetentionTimeGroup rtg;
        rtg.mass_diff_groups.reserve(finalObjects[i].size());
        for (int j = 0; j < finalObjects[i].size(); ++j) {
            // finalObjects[i][j] represents a MassDiffGroup
            MassDiffGroup mdg;
            mdg.paired_objects.reserve(finalObjects[i][j].size());
            mdg.rs_results.reserve(finalObjects[i][j].size());
            for (int k = 0; k < finalObjects[i][j].size(); ++k) {
                mdg.paired_objects.push_back(finalObjects[i][j][k]);
                mdg.rs_results.push_back(finalObjects[i][j][k].attributes.ele1.reactionSearchResults);
            }
            rtg.mass_diff_groups.push_back(mdg);
        }
        rtg.formulas = finalFormulas[i];
        rsd_results.rt_groups.push_back(rtg);
    }

    rsd_results_ = rsd_results;
    success_ = true;
}

ReactionSearchDecluster::RSDResult ReactionSearchDecluster::getRSDResult()
{
    return rsd_results_;
}

void ReactionSearchDecluster::constructPeakingInfo(vector<Element>& data)
{
    /*This function is to find isotope information from the data read in.
    Isotope is integrated in each piece of element.
    As far as the code is concerned, it is to convert data from string type to double type.*/
    for (int i = 0; i < data.size(); i++)
    {
        Element ele = data[i];
        double mzmed = stof(ele.getPropertyValue("mzmed"));
        double rt = stof(ele.getPropertyValue("rtmed"));
        double mean1 = stod(ele.getPropertyValue("dataset2_mean"));
        // Here 0 has no meaning to an element
        PairPeakingParameters peakingParameters = PairPeakingParameters(0, mzmed, rt, mean1);
        ele.peakParameters = peakingParameters;
        data[i] = ele;
    }
}

void ReactionSearchDecluster::isotopePairPeaking(
    vector<Element> data,
    PairPeakingParameters thresholds,
    vector<IsotopePair>& pairedData
) {
    /*This function is to find pairs from the input data base on a set of parameters.*/
    // Step 0: copy thresholds.
    double t_mzmed = thresholds.mzmed;
    double t_rtmed = thresholds.rtmed;
    double t_mean1 = thresholds.mean1;

    // Step 1: sort the input data based on input peaking info. Note: peaking info should constructed before entering this function.
    vector<Element> sortedElements = data;
    sortByPeakingInfo(sortedElements);
    vector<IsotopePair> pairs;

    // Output status
    for (int i = 0; i < sortedElements.size() - 1; i++)
    {
        Element ele = sortedElements[i];
        double Mm = ele.peakParameters.mzmed;
        double Me = Mm + thresholds.massDiff;
        double rtmed = ele.peakParameters.rtmed;
        double mean1 = ele.peakParameters.mean1;

        if (abs(mean1) < 0.000000001)
            continue;

        double k1 = 1000000;
        vector<Element> candidates;
        vector<double> factors;
        for (int j = i + 1; j < sortedElements.size(); j++)
        {
            Element candi = sortedElements[j];
            double Mm_candi = candi.peakParameters.mzmed;
            double accuracy = k1 * abs(Mm_candi - Me) / Me;
            if (Mm_candi > Me && accuracy > t_mzmed)
                break;
            // Filter 1: Mass accuracy
            if (accuracy > t_mzmed)
                continue;
            // Filter 2: rtmed
            double rtmed_candi = candi.peakParameters.rtmed;
            if (abs(rtmed_candi - rtmed) > t_rtmed)
                continue;
            // Filter 3: mean1

            double mean1_candi = candi.peakParameters.mean1;

            if (abs(mean1_candi) < 0.000000001)
                continue;

            double factor = max(mean1, mean1_candi) / min(mean1, mean1_candi);
            //if (factor > t_mean1)
            //	continue;

            if (factor > t_mean1)
                continue;

            candidates.push_back(candi);
            factors.push_back(factor);
        }

        if (candidates.size() > 0)
        {
            if (candidates.size() == 1)
            {
                pairs.push_back(IsotopePair(ele, candidates[0]));
            }
            else
            {
                // Find the one with least mean.
                int leastMean = 100;
                Element best = candidates[0];
                for (int j = 0; j < candidates.size(); j++)
                {
                    if (factors[j] < leastMean)
                    {
                        best = candidates[j];
                        leastMean = factors[j];
                    }
                }
                pairs.push_back(IsotopePair(ele, best));
            }
        }
    }
    pairedData = pairs;
}

void ReactionSearchDecluster::constructAdductList(
    vector<pair<string, double>>& positive,
    vector<pair<string, double>>& negative
)
{
    positive.push_back(make_pair("H", 1.007276));
    positive.push_back(make_pair("Na", 22.98922));
    positive.push_back(make_pair("K", 38.96316));
    positive.push_back(make_pair("NH4+", 18.03382));

    negative.push_back(make_pair("H", 1.007276));
    negative.push_back(make_pair("Cl-", 34.94861));
    negative.push_back(make_pair("CH3COO-", 59.013304));
}

void ReactionSearchDecluster::reactionDatabaseSearch(
    vector<IsotopePair> groups,
    vector<Element> database,
    ReactionSearchParameters reactionSearchParameters,
    vector<pair<string, double>> adductList,
    vector<vector<vector<vector<pair<int, Element>>>>>& hits,
    vector<vector<vector<double>>>& accuracyValues,
    int num_threads
) { 
    /*Need do searching for each adduct in adductList.*/
    int no_jobs_per_thread = groups.size() / num_threads;
    if (groups.size() % num_threads != 0)
        ++no_jobs_per_thread;

    // Temporary data storage
    vector<vector<vector<vector<vector<pair<int, Element>>>>>> hits_tmp(num_threads);
    vector<vector<vector<vector<double>>>> accuracyValues_tmp(num_threads);

    /*
    vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        int start = i * no_jobs_per_thread;
        int end = start + no_jobs_per_thread - 1 < groups.size() ? start + no_jobs_per_thread - 1 : groups.size() - 1;

        threads.push_back(std::thread(
            reactionDatabaseSearch_mt, std::ref(groups), start, end, database, reactionSearchParameters,
            adductList, std::ref(hits_tmp[i]), std::ref(accuracyValues_tmp[i])
        ));
    }

    for (int i = 0; i < num_threads; ++i) {
        threads[i].join();
    }*/

    vector<progress_t> progress(num_threads);
    vector<std::future<void>> futures;
    for (int i = 0; i < num_threads; ++i) {
        int start = i * no_jobs_per_thread;
        int end = start + no_jobs_per_thread - 1 < groups.size() ? start + no_jobs_per_thread - 1 : groups.size() - 1;
        futures.push_back(std::async(reactionDatabaseSearch_mt, std::ref(groups), start, end, database,reactionSearchParameters, adductList, std::ref(hits_tmp[i]), std::ref(accuracyValues_tmp[i]), std::ref(progress[i])));
    }

    int total_processed = 0;
    int current_progress = 0; // 0 out of 100
    while(total_processed != groups.size()) {
        total_processed = 0;
        for (int i = 0; i < progress.size(); ++i)
            total_processed += progress[i].value;
        current_progress = int(float(total_processed) / float(groups.size()) * 100);
        emit updateProgress("Reaction database search ...", current_progress); // Emit signal to update progress
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Add sleep to lower the cpu usage.
    }

    // Copy data
    for (int i = 0; i < num_threads; ++i) {
        hits.insert(hits.end(), hits_tmp[i].begin(), hits_tmp[i].end());
        accuracyValues.insert(accuracyValues.end(), accuracyValues_tmp[i].begin(), accuracyValues_tmp[i].end());
    }
}

void ReactionSearchDecluster::reactionDatabaseSearch_mt(
    vector<IsotopePair>& all_groups,
    int start, int end,
    vector<Element> database,
    ReactionSearchParameters reactionSearchParameters,
    vector<pair<string, double>> adductList,
    vector<vector<vector<vector<pair<int, Element>>>>>& hits,
    vector<vector<vector<double>>>& accuracyValues,
    progress_t& progress
) {
    /* m_prime is an user-input variable.
    For each isotope pair, calculate a mass deviation del according to:
    isPositive == true: del = mo - 1.007825 - m_prime
    isPositive == false: del = mo + 1.007825 - m_prime.
    Then, search for possible combinations based on del.*/

    mtx.lock();
    vector<IsotopePair> groups;
    groups.insert(groups.end(), all_groups.begin() + start, all_groups.begin() + end + 1);
    mtx.unlock();

    bool isPositive = reactionSearchParameters.isPositive;
    double m_prime = reactionSearchParameters.m_prime;
    double threshold = reactionSearchParameters.threshold;
    int num_combination = reactionSearchParameters.numCombination;

    vector<vector<vector<vector<pair<int, Element>>>>> hits_tmp;
    vector<vector<vector<double>>> accuracyValues_tmp;
    for (int i = 0; i < groups.size(); i++)
    {
        //cout << "Now processing Group ID: " << i << endl;
        IsotopePair group = groups[i];
        double mo = stof(group.ele1.getPropertyValue("mzmed"));

        vector<vector<vector<pair<int, Element>>>> results;
        vector<vector<double>> values;
        for (int j = 0; j < adductList.size(); j++)
        {
            //cout << j << "th->" << endl;
            string adductName = adductList[j].first;
            double adductMass = adductList[j].second;

            double del = 0;
            if (isPositive)
                del = mo - adductMass - m_prime;
            else
                del = mo + adductMass - m_prime;

            // Compare with possible combinations
            vector<vector<pair<int, Element>>> result;
            vector<double> massAccuracyValues;
            int maxNumElements = num_combination;
            combinationMassComparison(database, maxNumElements, mo, m_prime, del, threshold, result, massAccuracyValues);

            results.push_back(result);
            values.push_back(massAccuracyValues);
        }
        hits_tmp.push_back(results);
        accuracyValues_tmp.push_back(values);

        mtx.lock();
        progress.value = i + 1;
        mtx.unlock();
    }

    mtx.lock();
    hits = hits_tmp;
    accuracyValues = accuracyValues_tmp;
    mtx.unlock();
}

bool ReactionSearchDecluster::combinationMassComparison(
    vector<Element> database,
    int maxNumElements,
    double mo,
    double m_prime,
    double del,
    double threshold,
    vector<vector<pair<int, Element>>>& result,
    vector<double>& massAccuracyValues
) {
    // Sort database based on mass for searching performance improvement
    vector<Element> database_sorted = database;
    if (sortDatabseByMass(database_sorted))
        database = database_sorted;

    vector<int> max_appearance;
    for (int i = 0; i < database.size(); i++)
    {
        //int appearance = stod(database[i].getPropertyValue("maxnum"));
        double monoisotopicMass = stof(database[i].getPropertyValue("Monoisotopic Mass Change"));
        int appearance = int((mo - m_prime) / monoisotopicMass);
        if (appearance < 0)
            appearance = 0;
        max_appearance.push_back(appearance);
    }

    for (int i = 1; i <= maxNumElements; i++)
    {
        vector<pair<int, Element>> searched;
        combinationMassComparison(database, max_appearance, i, del, threshold, searched, result, massAccuracyValues);
    }

    return true;
}

bool ReactionSearchDecluster::combinationMassComparison(
    vector<Element> data,
    vector<int> max_appearance,
    int num,
    double del,
    double threshold,
    vector<pair<int, Element>> searched,
    vector<vector<pair<int, Element>>>& result,
    vector<double>& massAccuracyValues
) {
    if (num == 0)
        return false;

    if (data.size() == 1)
    {
        // Only one element left
        int max_appear = max_appearance[0];
        if (max_appear < num)
            return false;
        else
        {
            vector<pair<int, Element>> combination = searched;
            combination.push_back(make_pair(num, data[0]));
            //combinations.push_back(combination);
            double combinationMass = ComputeMass(combination);
            if (MassFilter(del, combinationMass, threshold))
            {
                double accuracy = ComputeMassAccuracy(del, combinationMass);
                massAccuracyValues.push_back(accuracy);
                result.push_back(combination);
            }
            return true;
        }
    }

    int max_appear = max_appearance[0];
    for (int j = 0; j < max_appear + 1; j++)
    {
        vector<Element> new_data = data;
        new_data.erase(new_data.begin());
        vector<int> new_appearance = max_appearance;
        new_appearance.erase(new_appearance.begin());
        int new_num = num - j;
        if (new_num == 0)
        {
            vector<pair<int, Element>> combination = searched;
            combination.push_back(make_pair(j, data[0]));
            //combinations.push_back(result);
            double combinationMass = ComputeMass(combination);
            if (MassFilter(del, combinationMass, threshold))
            {
                double accuracy = ComputeMassAccuracy(del, combinationMass);
                massAccuracyValues.push_back(accuracy);
                result.push_back(combination);
            }
            break;
        }
        else if (new_num > 0)
        {
            vector<pair<int, Element>> new_searched = searched;
            if (j != 0)
                new_searched.push_back(make_pair(j, data[0]));

            // Check for fast searching. If current combination already has a mass greater than del
            // and the mass accuracy is not met, we don't have to continue further searching as the
            // input data is sorted so the total mass and mass diff are greater.
            double curr_mass = ComputeMass(new_searched);
            if (curr_mass > del && !MassFilter(del, curr_mass, threshold))
                break;

            combinationMassComparison(new_data, new_appearance, new_num, del, threshold, new_searched, result, massAccuracyValues);
        }
        else
        {
            // new_num is negative.
            break;
        }
    }
}

bool ReactionSearchDecluster::MassFilter(double observed, double theoratical, double threshold)
{
    double accuracy = ComputeMassAccuracy(observed, theoratical);
    if (accuracy <= threshold)
        return true;
    else
        return false;
}

double ReactionSearchDecluster::ComputeMass(vector<pair<int, Element>> combination)
{
    double mass = 0;
    for (int m = 0; m < combination.size(); m++)
    {
        mass = mass + combination[m].first * stof(combination[m].second.getPropertyValue("Monoisotopic Mass Change"));
    }
    return mass;
}

double ReactionSearchDecluster::ComputeMassAccuracy(double observed, double theoratical)
{
    double k1 = 1000000;
    double accuracy = k1 * abs(observed - theoratical) / theoratical;
    return accuracy;
}

void ReactionSearchDecluster::retentionTimeBasedGrouping(
    vector<IsotopePair> data,
    double t_rtmed,
    vector<vector<IsotopePair>>& groups
) {
    /*t_rtmed is the treshold of retention time.*/
    groups.reserve(data.size());
    vector<IsotopePair> sortedPairs;
    //sort(sortedPairs.begin(), sortedPairs.end(), IsotopePairRtComparator()); // Sort isotope pairs by retention time.
    for (int i = 0; i < data.size(); i++)
    {
        IsotopePair current = data[i];

        vector<IsotopePair>::iterator iter = sortedPairs.begin();
        for (; iter != sortedPairs.end(); ++iter)
        {
            IsotopePair tgt = *iter;
            if (current.getMinRt() < tgt.getMinRt())
                break;
            else if (current.getMinRt() > tgt.getMinRt())
                continue;
            else
            {
                if (current.getMaxRt() < tgt.getMaxRt())
                    break;
                else if (current.getMaxRt() > tgt.getMaxRt())
                    continue;
                else
                    break;
            }
        }
        sortedPairs.insert(iter, current);
    }

    for (int i = 0; i < sortedPairs.size();)
    {
        vector<IsotopePair> group;
        group.push_back(sortedPairs[i]);
        double base = sortedPairs[i].getMinRt();
        int j = i + 1;
        for (; j < sortedPairs.size(); j++)
        {
            if (abs(sortedPairs[j].getMinRt() - base) <= t_rtmed)
                group.push_back(sortedPairs[j]);
            else
                break;
        }
        i = j;
        groups.push_back(group);
    }
}

void ReactionSearchDecluster::constructStructInfo(
    vector<Element> data,
    vector<StructureInfo>& structureInfo
) {
    /*This function is to construct struct info from input data. For each piece of structure info, it has:
    1. A structure, e.g. M-H, M-NH4, etc,
    2. Coefficients of the mass formulation.*/
    for (int i = 0; i < data.size(); i++)
    {
        Element ele = data[i];
        string structure = ele.getPropertyValue("Structure");
        double numerator = stod(ele.getPropertyValue("Numerator"));
        double denominator = stod(ele.getPropertyValue("Denominator"));
        double bias = stod(ele.getPropertyValue("Bias"));
        StructureInfo info = StructureInfo(structure, (numerator / denominator), bias);
        structureInfo.push_back(info);
    }
}

void ReactionSearchDecluster::generateAdductList(
    vector<Element> data,
    vector<StructureInfo>& adductList
) {
    constructStructInfo(data, adductList);
}

void ReactionSearchDecluster::adductDeclustering(
    vector<vector<IsotopePair>> groups,
    vector<StructureInfo> adductList,
    vector<StructureInfo> positive,
    vector<StructureInfo> negative,
    bool isPositive,
    double adduct_massAccuracy,
    vector<vector<vector<IsotopePairObject>>>& finalObjects,
    vector<vector<vector<string>>>& finalFormulas
) {
    /*In this function, each group of isotope pair is processed.*/
    vector<StructureInfo> dictionary = isPositive ? positive : negative;
    vector<vector<vector<IsotopePairObject>>> results_objects;
    vector<vector<vector<string>>> results_formulate;

    for (int i = 0; i < groups.size(); i++)
    {
        vector<IsotopePair> group = groups[i];

        // Step 1: Isotope feature labelling.
        vector<IsotopePairObject> objects;
        isotopePairLabelling(group, objects);

        // Step 2: Group objects by mass difference.
        vector<vector<IsotopePairObject>> objectGroups;
        groupIsotopeObjectsByMassDifference(objects, objectGroups, true);

        // Step 3: Process each subgroup find candidates for further searching.
        // Update objectGroups
        vector<vector<IsotopePairObject>> updatedObjectGroups;
        for (int j = 0; j < objectGroups.size(); j++)
        {
            IsotopePairObject first = objectGroups[j][0];
            bool isHighest = true;
            // Check if the first one is the one with the highest maxint.
            for (int k = 1; k < objectGroups[j].size(); j++)
            {
                if (stof(first.obj.getPropertyValue("maxint")) < stof(objectGroups[j][k].obj.getPropertyValue("maxint")))
                {
                    isHighest = false;
                    break;
                }
            }
            if (isHighest)
                updatedObjectGroups.push_back(objectGroups[j]);
            else
            {
                for (int k = 0; k < objectGroups[j].size(); k++)
                {
                    vector<IsotopePairObject> temp;
                    temp.push_back(objectGroups[j][k]);
                    updatedObjectGroups.push_back(temp);
                }
            }
        }
        objectGroups = updatedObjectGroups;

        // Step 4: Searching. Get possible results.
        vector<vector<string>> results;
        int maxhit = 0;
        for (int j = 0; j < objectGroups.size(); j++)
        {
            IsotopePairObject current = objectGroups[j][0];
            double currentMass = current.obj.peakParameters.mzmed;
            // Try each adduct
            for (int k = 0; k < adductList.size(); k++)
            {
                // e.g. adductList[j] is M-H, newMass is the mass of M.
                double M_mass = adductList[k].invCalculate(currentMass);
                vector<double> massList(dictionary.size(), 0);
                for (int m = 0; m < dictionary.size(); m++)
                    massList[m] = dictionary[m].calculate(M_mass);
                vector<string> result;
                int hit = 0;
                for (int m = 0; m < objectGroups.size(); m++)
                {
                    string s = "";
                    if (m == j)
                        s = adductList[k].getStructure();
                    else
                    {
                        for (int n = 0; n < massList.size(); n++)
                        {
                            if (MassFilter(objectGroups[m][0].obj.peakParameters.mzmed, massList[n], adduct_massAccuracy))
                            {
                                ++hit;
                                s = dictionary[n].getStructure();
                                break;
                            }
                        }
                    }
                    result.push_back(s);
                }
                if (hit == 0)
                    continue;

                // Check if this possibility has been added.
                bool isAdded = false;
                for (int m = 0; m < results.size(); m++)
                {
                    bool sameResult = true;
                    for (int n = 0; n < results[m].size(); n++)
                    {
                        if (results[m][n].compare(result[n]) != 0)
                        {
                            sameResult = false;
                            break;
                        }
                    }
                    if (sameResult)
                    {
                        isAdded = true;
                        break;
                    }
                }

                if (isAdded)
                    continue;

                if (hit == maxhit)
                    results.push_back(result);
                else if (hit > maxhit)
                {
                    results.clear();
                    results.push_back(result);
                    maxhit = hit;
                }
            }
        }

        // Step 5: save results.
        results_objects.push_back(objectGroups);
        results_formulate.push_back(results);
    }

    finalObjects = results_objects;
    finalFormulas = results_formulate;
}

void ReactionSearchDecluster::isotopePairLabelling(
    vector<IsotopePair>& data,
    vector<IsotopePairObject>& objects
) {
    /*In this function, each isotope pair will be checked.
    If their mass and maxint meet certain conditions, they will be labelling.
    In this function, each isotope pair will be converted to an isotope object.
    Each object contains information: pair information and which element is used for further computation.*/
    for (int i = 0; i < data.size(); i++)
    {
        IsotopePair isotopePair = data[i];
        Element ele = isotopePair.ele1.peakParameters.mzmed <= isotopePair.ele2.peakParameters.mzmed ?
            isotopePair.ele1 : isotopePair.ele2;
        objects.push_back(IsotopePairObject(ele, isotopePair));
    }
}

void ReactionSearchDecluster::groupIsotopeObjectsByMassDifference(
    vector<IsotopePairObject> objects,
    vector<vector<IsotopePairObject>>& groups,
    bool sortByMass
) {
    /*This function is to group objects based on mass difference. If two objects' mass difference is 1.0033, or 2.0066, or 3.0099,
    they should belong to one group.
    If sortByMass is true, the elements in each group of groups are sorted by mass.*/
    double threshold = 0.0005;

    vector<vector<double>> massDifference;
    for (int i = 0; i < objects.size(); i++)
    {
        vector<double> temp(objects.size(), 0);
        massDifference.push_back(temp);
    }
    // Calculate mass difference.
    for (int i = 0; i < objects.size() - 1; i++)
    {
        IsotopePairObject src = objects[i];
        for (int j = i + 1; j < objects.size(); j++)
        {
            IsotopePairObject tgt = objects[j];
            double difference = src.obj.peakParameters.mzmed - tgt.obj.peakParameters.mzmed;
            massDifference[i][j] = difference;
            massDifference[j][i] = -difference;
        }
    }
    // Group based on mass difference.
    vector<int> groupIndicators;
    for (int i = 0; i < objects.size(); i++)
        groupIndicators.push_back(i);
    for (int i = 0; i < massDifference.size() - 1; i++)
    {
        for (int j = i + 1; j < massDifference.size(); j++)
        {
            double difference = massDifference[i][j];
            if (abs(difference - 1.0033) < threshold ||
                abs(difference - 2.0066) < threshold ||
                abs(difference - 3.0099) < threshold)
            {
                // They should belong to the same group.
                int newGroupID = min(groupIndicators[i], groupIndicators[j]);
                for (int k = 0; k < groupIndicators.size(); k++)
                {
                    if (groupIndicators[k] == groupIndicators[i] || groupIndicators[k] == groupIndicators[j])
                        groupIndicators[k] = newGroupID;
                }
            }
        }
    }
    groups.resize(objects.size());
    for (int i = 0; i < groupIndicators.size(); i++)
        groups[groupIndicators[i]].push_back(objects[i]);
    for (int i = 0; i < groups.size(); i++)
    {
        if (groups[i].size() == 0)
        {
            groups.erase(groups.begin() + i);
            --i;
        }
    }

    if (sortByMass)
    {
        for (int i = 0; i < groups.size(); i++)
            sort(groups[i].begin(), groups[i].end(), IsotopePairObjectMassComparator());
    }
}

void ReactionSearchDecluster::sortByPeakingInfo(vector<Element>& data)
{
    sort(data.begin(), data.end(), ElementPeakingInfoComparator());

    // Checking function
    for (int i = 0; i < data.size() - 1; i++)
    {
        assert(data[i + 1].peakParameters.mzmed >= data[i].peakParameters.mzmed);
    }
}

bool ReactionSearchDecluster::sortDatabseByMass(vector<Element> &database)
{
    vector<pair<int, double>> index_mass(database.size());
    for (int i = 0; i < database.size(); ++i) {
        index_mass[i].first = i;
        string mono_mass_change = database[i].getPropertyValue("Monoisotopic Mass Change");
        if (mono_mass_change.empty())
            return false;
        index_mass[i].second = stof(mono_mass_change);
    }

    sort(index_mass.begin(), index_mass.end(), [](pair<int, double> &left, pair<int, double> &right) {
        return left.second < right.second;
    });

    vector<Element> database_sorted(database.size());
    for (int i = 0; i < index_mass.size(); ++i)
        database_sorted[i] = database[index_mass[i].first];
    database.clear();
    database = database_sorted;
    return true;
}
