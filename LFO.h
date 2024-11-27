/*
  ==============================================================================

    LFO.h
    Created: 5 Sep 2024 4:26:48pm
    Author:  benny

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
class Node : public juce::Component
{
public:
    Node() : constrainer(), prev(nullptr), next(nullptr), fixedX(false), posX(0)
    {
        constrainer.setMinimumOnscreenAmounts(10, 10, 10, 10);
        
    }
    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::green);
        g.drawEllipse(2.5f, 2.5f, 5.f, 5.f, 2.f);
    }

    void resized() override
    {

    }
    void mouseDown(const juce::MouseEvent& e) override
    {
        posX = getX();
        dragger.startDraggingComponent(this, e);
    }
    void mouseDrag(const juce::MouseEvent& e) override
    {
        dragger.dragComponent(this, e, &constrainer);
        if (next)
        {
            if (getPosition().getX() > next->getPosition().getX())
            {
                setTopLeftPosition(next->getPosition().getX(), getY());
            }
        }
        if (prev)
        {
            if (getPosition().getX() < prev->getPosition().getX())
            {
                setTopLeftPosition(prev->getPosition().getX(), getY());
            }
        }
        if (fixedX)
        {
            setTopLeftPosition(posX, getY());
        }
    }

    void mouseDoubleClick(const juce::MouseEvent& e) override
    {
        if (!fixedX)
        {
            //while the delete function does this, if we tell
            //the parent that it should repaint inside the 
            //delete function, the program will decide to 
            //repaint over and over and over again upon
            //closing the program, reaching gigabytes of 
            //memory usage within seconds.
            //to fix this, we remove this node from the 
            //linked list, then call the parent so that 
            //it redraws correctly,
            //then deletes itself.
            if (prev)
                prev->setNext(next);
            if (next)
                next->setPrev(prev);
            getParentComponent()->childBoundsChanged(this);
            delete this;
        }
            
    }
    Node* getNext() { return next; }
    Node* getPrev() { return prev; }
    void setNext(Node* n) { next = n; }
    void setPrev(Node* n) { prev = n; }
    void setFixedX(bool f)
    {
        fixedX = f;
        
    }
    bool getFixedX() { return fixedX; }
    
    void insertNext(Node* n)
    {
        jassert(n);
        n->setPrev(this);
        n->setNext(next);
        next->setPrev(n);
        next = n;
        
    }
    void insertPrev(Node* n)
    {
        jassert(n);
        n->setNext(this);
        n->setPrev(prev);
        prev->setNext(n);
        prev = n;
    }
    ~Node()
    {
        if (prev)
            prev->setNext(next);
        if (next)
            next->setPrev(prev);
    }
private:
    juce::ComponentDragger dragger;
    juce::ComponentBoundsConstrainer constrainer;
    bool fixedX;
    int posX;
    Node* prev;
    Node* next;
};

class LFO : public juce::Component
{
public:
    struct Tempo
    {
        bool isFraction;
        float seconds;
        int numerator;
        int denominator;
        float bpm;
        float sampleRate;
        void advance(float* posToAdvance)
        {
            if (isFraction)
            {
                /*float onePerSecond = 1.f / sampleRate;
                float onePerBeat = onePerSecond * bpm / 60.f;
                float amountToAdvance = onePerBeat * numerator / denominator;*/
                // Step 1: Calculate the duration of a beat in seconds
                float beat_duration = (60.0f / bpm) * (4.f / denominator);

                // Step 2: Calculate the duration of a bar (in seconds)
                float bar_duration = beat_duration * numerator;

                // Step 3: Calculate how many samples occur in one bar
                float samples_per_bar = sampleRate * bar_duration;

                // Step 4: Determine the increment per sample
                float increment_per_sample = 1.0f / samples_per_bar;
                *posToAdvance += increment_per_sample * samples_per_bar;
                if (*posToAdvance > 1.f)
                    *posToAdvance -= 1.f;
            }
        }
    };

    LFO() : tempo({true, 0.f, 1, 4, 120.f, 48000.f}), beginNode(std::make_unique<Node>()), endNode(std::make_unique<Node>()), currentVal(0.f)
    {
        beginNode->setNext(endNode.get());
        beginNode->setPrev(endNode.get());
        endNode->setNext(beginNode.get());
        endNode->setPrev(beginNode.get());

        addAndMakeVisible(beginNode.get());
        addAndMakeVisible(endNode.get());
    }
    LFO(Tempo tempo) : tempo(tempo), currentVal(0.f)
    {
        beginNode->setNext(endNode.get());
        beginNode->setPrev(endNode.get());
        endNode->setNext(beginNode.get());
        endNode->setPrev(beginNode.get());

        addAndMakeVisible(beginNode.get());
        addAndMakeVisible(endNode.get());
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::black);
        g.fillAll();
        g.setColour(juce::Colours::darkgreen);
        g.fillPath(curve);
        g.drawLine(currentVal * getWidth(), getHeight(), currentVal * getWidth(), 0.f, 1);
        //g.drawText(juce::String(currentVal), 0, 0, 40, 20, juce::Justification::centredLeft);
    }

    void mouseDoubleClick(const juce::MouseEvent& e)
    {
        Node* newNode = new Node();
        
        newNode->setSize(10, 10);
        newNode->setCentrePosition(e.getPosition().getX(), e.getPosition().getY());
        
        
        Node* currentNode = beginNode.get();
        while (newNode->getPrev() == nullptr)
        {
            if (currentNode->getPosition().getX() <= newNode->getPosition().getX())
            {
                if (currentNode->getNext()->getPosition().getX() > newNode->getPosition().getX())
                {
                    currentNode->insertNext(newNode);
                }
            }
            currentNode = currentNode->getNext();
        }
        
        addAndMakeVisible(newNode);
        resetCurve();
        
    }

    void childBoundsChanged(Component* child) override
    {
        resetCurve();
        
    }
    void resized() override
    {
        beginNode->setBounds(0, getHeight() - 10, 10, 10);
        beginNode->setFixedX(true);
        endNode->setBounds(getWidth() - 10, getHeight() - 10, 10, 10);
        endNode->setFixedX(true);
    }
    
    void resetCurve()
    {
        Node* currentNode = beginNode.get();
        curve.clear();
        while (currentNode != endNode.get())
        {
            curve.startNewSubPath(currentNode->getBounds().getCentre().toFloat());
            curve.quadraticTo(currentNode->getNext()->getBounds().getCentre().toFloat(), currentNode->getNext()->getBounds().getCentre().toFloat());
            currentNode = currentNode->getNext();
        }
        juce::PathStrokeType stroke(2.f);
        stroke.createStrokedPath(curve, curve);
        repaint();
    }
    float getValue(float* position)
    {
        float pos = *position;
        currentVal = pos;
        tempo.advance(position);
        juce::Path tempPath(curve);
        auto it = juce::PathFlatteningIterator(tempPath, juce::AffineTransform(), 1.f);
        bool exists = true;
        while (exists)
        {
            if (it.x1 > pos * getWidth())
                return 1.f - (it.y1 + (pos * (float)getWidth() - it.x1) * (it.y2 - it.y1) / (it.x2 - it.x1)) / (float)getHeight();
            exists = it.next();
        }
        return 0.f; // juce::AffineTransform::scale(1.f / getWidth())
    }

    void mouseMove(const juce::MouseEvent& e) override
    {
        //currentVal = getValue(e.getPosition().toFloat().getX() / (float)getWidth());
        //repaint();
    }

    

    ~LFO()
    {
        Node* current = beginNode->getNext();
        while (current != endNode.get())
        {
            delete current;
            current = beginNode->getNext();
        }
    }
private:
    float currentVal;
    Tempo tempo;
    juce::Path curve;
    std::unique_ptr<Node> beginNode;
    std::unique_ptr<Node> endNode;
};

class LFOAttachment
{
public:
    LFOAttachment(LFO* lfoToAttach) : position(0.f), amount(1.f), lfo(lfoToAttach)
    {

    }
    void attachToLFO(LFO*lfoToAttach)
    {
        lfo = lfoToAttach;
    }

    float getValue()
    {
        return lfo->getValue(&position);
    }
    
    void reset() { position = 0.f; }
private:
    float position;
    float amount;
    LFO* lfo;
};


