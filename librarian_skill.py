# -*- coding: utf-8 -*-
""" simple fact sample app """
import boto3

SKILL_NAME = "Space Facts"
GET_FACT_MESSAGE = "Here's your fact: "
HELP_MESSAGE = "You can say tell me a space fact, or, you can say exit... What can I help you with?"
HELP_REPROMPT = "What can I help you with?"
STOP_MESSAGE = "Goodbye!"
FALLBACK_MESSAGE = "The Space Facts skill can't help you with that.  It can help you discover facts about space if you say tell me a space fact. What can I help you with?"
FALLBACK_REPROMPT = 'What can I help you with?'

# --------------- App entry point -----------------

def lambda_handler(event, context):
    """  App entry point  """

    print(event)

    if event['session']['new']:
        print("on_session_started")

    if event['request']['type'] == "LaunchRequest":
        return on_launch(event['request'])
    elif event['request']['type'] == "IntentRequest":
        return on_intent(event['request'], event['session'])
    elif event['request']['type'] == "SessionEndedRequest":
         print("on_session_ended")


# --------------- Response handlers -----------------

def on_intent(request, session):
    """ called on receipt of an Intent  """

    intent_name = request['intent']['name']
    for val in request:
        print val
        print request[val]
    if request['intent']['slots']['acronym']['resolutions']['resolutionsPerAuthority'][0]['status']['code'] == "ER_SUCCESS_MATCH":
        slot = request['intent']['slots']['acronym']['resolutions']['resolutionsPerAuthority'][0]['values'][0]['value']['name']
    else:
        slot = request['intent']['slots']['acronym']['value']
    print slot

    # process the intents
    if intent_name == "meaningIntent":
        return acronym_response(slot)
    elif intent_name == "AMAZON.HelpIntent":
        return get_help_response()
    elif intent_name == "AMAZON.StopIntent":
        return get_stop_response()
    elif intent_name == "AMAZON.CancelIntent":
        return get_stop_response()
    elif intent_name == "AMAZON.FallbackIntent":
        return get_fallback_response()
    else:
        print("invalid Intent reply with help")
        return get_help_response()

def acronym_response(slotval):
    """ get and return a random fact """
    print "Inside acronym response"

    answers = []

    s3 = boto3.client('s3')
    obj = s3.get_object(Bucket="uai3026771-alexa-interns", Key="alex-jeff-dhruv.csv")
    data = obj[u'Body'].read().split("\n")
    for line in data:
        current_line = line.split(",")
        if len(current_line) > 2:
            if current_line[2] == slotval.upper():
                answers.append(current_line[3])
                print current_line[3]
    say = slotval + " stands for "
    for i in range(0,len(answers)):
        if i != 0:
            say += " or "
        say += answers[i] + ". \n"
    
    return response(speech_response(say, True))

def get_help_response():

    speech_message = HELP_MESSAGE
    return response(speech_response_prompt(speech_message,
                                                       speech_message, False))
def get_launch_response():

    return response(speech_response("Welcome to the acronym bot. Ask me what something stands for.",False))

def get_stop_response():

    speech_output = STOP_MESSAGE
    return response(speech_response(speech_output, True))

def get_fallback_response():

    speech_output = FALLBACK_MESSAGE
    return response(speech_response(speech_output, False))

def on_launch(request):
    return get_launch_response()


# --------------- Speech response handlers -----------------

def speech_response(output, endsession):
    """  create a simple json response  """
    return {
        'outputSpeech': {
            'type': 'PlainText',
            'text': output
        },
        'shouldEndSession': endsession
    }

def dialog_response(endsession):
    """  create a simple json response with card """

    return {
        'version': '1.0',
        'response':{
            'directives': [
                {
                    'type': 'Dialog.Delegate'
                }
            ],
            'shouldEndSession': endsession
        }
    }

def speech_response_with_card(title, output, cardcontent, endsession):
    """  create a simple json response with card """

    return {
        'card': {
            'type': 'Simple',
            'title': title,
            'content': cardcontent
        },
        'outputSpeech': {
            'type': 'PlainText',
            'text': output
        },
        'shouldEndSession': endsession
    }

def response_ssml_text_and_prompt(output, endsession, reprompt_text):
    """ create a Ssml response with prompt  """

    return {
        'outputSpeech': {
            'type': 'SSML',
            'ssml': "<speak>" +output +"</speak>"
        },
        'reprompt': {
            'outputSpeech': {
                'type': 'SSML',
                'ssml': "<speak>" +reprompt_text +"</speak>"
            }
        },
        'shouldEndSession': endsession
    }

def speech_response_prompt(output, reprompt_text, endsession):
    """ create a simple json response with a prompt """

    return {
        'outputSpeech': {
            'type': 'PlainText',
            'text': output
        },
        'reprompt': {
            'outputSpeech': {
                'type': 'PlainText',
                'text': reprompt_text
            }
        },
        'shouldEndSession': endsession
    }

def response(speech_message):
    """ create a simple json response  """
    return {
        'version': '1.0',
        'response': speech_message
    }