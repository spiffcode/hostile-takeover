import os
import command
import serverinfo
import accounts

from google.appengine.ext.webapp import template

class Drain(command.Command):
    def have_access(self):
        account = accounts.account()
        if not account or not account.DRAIN_ACCESS_RIGHT:
            return False
        return True

    def finish_get(self, template_values):
        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'drain.xhtml')
        self.response.out.write(template.render(path, template_values))

    def finish_post(self, selected, template_values):
        drain_command = None
        if self.request.get('drain'):
            drain_command = 'drain'
        if self.request.get('undrain'):
            drain_command = 'undrain'

        errors = []
        if len(selected) == 0:
            errors.append('Must select at least one server.')
        if drain_command == None:
            errors.append('Must select drain or undrain.')
        else:
            for info in selected:
                serverinfo.ServerInfo.send_command(info, '{"command": "%s"}' % drain_command)
                errors.append('Server %s sent %s command.' % (info['name'], drain_command))

        template_values['errors'] = errors

        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'drain.xhtml')
        self.response.out.write(template.render(path, template_values))
               
