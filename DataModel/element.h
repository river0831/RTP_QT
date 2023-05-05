#ifndef ELEMENT_H
#define ELEMENT_H

#include <vector>
#include <string>
#include <map>
#include <algorithm>
using namespace std;

struct PairPeakingParameters
{
    double massDiff;
    double mzmed;
    double rtmed;
    double mean1;

    PairPeakingParameters(double mass_difference = 0, double mass = 0, double retention_time = 0, double dataset_mean1 = 0)
    {
        massDiff = mass_difference;
        mzmed = mass;
        rtmed = retention_time;
        mean1 = dataset_mean1;
    }
};

struct ReactionSearchParameters
{
    bool isPositive;
    double threshold;
    double m_prime;
    int numCombination;

    ReactionSearchParameters(
        bool v_positive = true,
        double v_threshold = 0,
        double v_m_prime = 0,
        int v_numCombination = 1
    ) {
        isPositive = v_positive;
        threshold = v_threshold;
        m_prime = v_m_prime;
        numCombination = v_numCombination;
    }
};

struct ReactionSearchResult
{
    string type; // 'M+H', 'M+Na', 'M+K', or 'M+NH4'
    string value;
    double accuracy;
    double sum;
    vector<pair<int, string>> combination;

    ReactionSearchResult(
        string Type,
        string Value,
        double Accuracy,
        double Sum,
        vector<pair<int, string>> Combination
    ) {
        type = Type;
        value = Value;
        accuracy = Accuracy;
        sum = Sum;
        combination = Combination;
    }
};

class Element
{
private:
    string m_name;
    vector<string> m_genes;
    vector<string> m_hits;
    float m_rate;
    map<string, double> m_property;
    map<string, string> m_allProperties;
    string m_rawLine;
    vector<Element> m_detects;
    vector<string> m_KEGG;
    string m_entry;
    string m_EC;

public:
    double mzmed;
    double rtmed;
    double mean1;
    PairPeakingParameters peakParameters;
    vector<ReactionSearchResult> reactionSearchResults;

public:
    Element()
    {
        m_rate = 0;
        mzmed = 0;
        rtmed = 0;
        mean1 = 0;
        m_entry = "";
        m_EC = "";
        m_rawLine = "";
        //peakParameters = PairPeakingParameters(0, 0, 0);
    }

    void setKEGG_GENES(vector<string> genes)
    {
        m_KEGG = genes;
    }

    vector<string> getKEGG_GENES()
    {
        return m_KEGG;
    }

    void setEntry(string entry)
    {
        m_entry = entry;
    }

    string getEntry()
    {
        return m_entry;
    }

    void setEC(string ec)
    {
        m_EC = ec;
    }

    string getEC()
    {
        return m_EC;
    }

    string getName()
    {
        return m_name;
    }

    void setRawLine(string s)
    {
        m_rawLine = s;
    }

    string getRawLine()
    {
        return m_rawLine;
    }

    vector<string> getHits()
    {
        return m_hits;
    }

    float getRates()
    {
        return m_rate;
    }

    void setName(string name)
    {
        m_name = name;
    }

    void setGenes(vector<string> genes)
    {
        m_genes = genes;
    }

    void addGene(string gene)
    {
        m_genes.push_back(gene);
    }

    void findHits(vector<string> target)
    {
        for (int i = 0; i<m_genes.size(); i++)
        {
            string s = m_genes[i];
            if (find(target.begin(), target.end(), s) != target.end())
            {
                m_hits.push_back(s);
            }
        }
        m_rate = float(m_hits.size()) / float(target.size());
    }

    void addProperty(string s, double value)
    {
        m_property.insert(make_pair(s, value));
    }

    void addProperty(string s, string value)
    {
        m_allProperties.insert(make_pair(s, value));
    }

    double propertyValue(string s)
    {
        return m_property[s];
    }

    string getPropertyValue(string s)
    {
        map<string, string>::iterator iter = m_allProperties.find(s);
        if (iter == m_allProperties.end())
            return "";
        else
            return iter->second;
    }

    void addDetect(Element ele)
    {
        m_detects.push_back(ele);
    }

    vector<Element> getDetects()
    {
        return m_detects;
    }
};

#endif // ELEMENT_H
