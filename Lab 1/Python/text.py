import smtplib
import ssl
carriers = {
	'att':    '@mms.att.net',
	'tmobile':' @tmomail.net',
	'verizon':  '@vtext.com',
	'sprint':   '@page.nextel.com'
}

def send(message):
        # Replace the number with your own, or consider using an argument\dict for multiple people.
	#to_number = '319-899-9264{}'.format(carriers['verizon'])
	auth = ('sam.morgan44@gmail.com', 'samikool44')

	s = smtplib.SMTP('smtp.gmail.com', 587) 

	s.starttls() 

	s.login(auth[0], auth[1])

	message = "Message_you_need_to_send"

	s.sendmail("sam.morgan44@gmail.com", "sam.morgan44@gmail.com", message) 

	s.quit() 

send("heres a message")
	