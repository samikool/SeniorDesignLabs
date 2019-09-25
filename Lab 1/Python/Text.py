# Download the helper library from https://www.twilio.com/docs/python/install
from twilio.rest import Client


# Your Account Sid and Auth Token from twilio.com/console
# DANGER! This is insecure. See http://twil.io/secure
account_sid = 'AC073d9e745d5a5da8e7fbe7bbfba84102'
auth_token = 'd2031f6ed8a3aeb15b1666628c076ae9'
client = Client(account_sid, auth_token)

message = client.messages.create(body="Join Earth's mightiest heroes. Like Kevin Bacon.",from_='+13092048749',to='+13092359608')

print(message.sid)