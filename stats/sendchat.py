import os
import command
import serverinfo
import accounts

from google.appengine.ext.webapp import template

class SendChat(command.Command):
    def have_access(self):
        account = accounts.account()
        if not account or not account.SEND_CHAT_ACCESS_RIGHT:
            return False
        return True

    def finish_get(self, template_values):
        template_values['name'] = 'From Admin'
        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'sendchat.xhtml')
        self.response.out.write(template.render(path, template_values))

    def finish_post(self, selected, template_values):
        name = self.request.get('name')
        message = self.request.get('message')

        errors = []
        if len(selected) == 0:
            errors.append('Must select at least one server.')

        if not name or not message:
            errors.append('Must have non-empty name and message.')
        
        if len(errors) == 0:
            for info in selected:
                command = '{"command": "chat", "name": "%s", "message": "%s"}' % (name, message)
                serverinfo.ServerInfo.send_command(info, command)
                errors.append('Message "%s: %s" successfully sent to server %s.' % (name, message, info['name']))
            message = ''

        template_values['name'] = name
        template_values['message'] = message
        template_values['errors'] = errors

        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'sendchat.xhtml')
        self.response.out.write(template.render(path, template_values))

